// Copyright (c) 2020, BlockProject
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//     * Neither the name of BlockProject nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#define BP_COMPAT_2_X

#include "Engine/AssetManager.hpp"
#include <Framework/ParseException.hpp>

//Force compilers like MSVC to generate symbols for the rendering engine part
#ifdef WINDOWS
    #include "Engine/Driver/IRenderEngine.hpp"
    #include "Engine/SimpleAsset.hpp"
class BP3D_API SimpleAsset; //This forces MSVC to write the symbols inside of BP3D.dll
#endif

using namespace bp3d;

void AssetManager::Add(const bpf::String &vpath, const bpf::String &url)
{
    _log.Info("Loading asset '[]' with url '[]'...", vpath, url);
    auto arr = url.Explode(',');
    if (arr.Size() != 2)
    {
        _log.Error("Could not load asset '[]': incorrect asset url format", vpath);
        return;
    }
    auto format = arr[0];
    auto location = arr[1];
    if (GetProvider(format) == Null)
    {
        _log.Error("Could not load asset '[]': no installed provider matches asset format ([])", vpath, format);
        return;
    }
    try
    {
        auto ptr = GetProvider(format)->Create(location);
        if (ptr == Null)
        {
            _log.Error("Could not load asset '[]': IAssetProvider failure", vpath);
            return;
        }
        _thread.Add(vpath, std::move(ptr));
        if (_thread.GetState() != bpf::system::Thread::RUNNING)
        {
            _thread.Join();
            _thread.Start();
        }
    }
    catch (const bpf::RuntimeException &ex)
    {
        _log.Error("Could not load asset '[]': an unhandled exception has occured", vpath);
        _log.Error("        > []: []", ex.Type(), ex.Message());
    }
}

bpf::io::File AssetManager::GetAssetPath(const bpf::system::Paths &paths, const bpf::String &location)
{
    auto path = location.Replace("%Cache%", paths.CacheDir.Path())
                    .Replace("%App%", paths.AppRoot.Path())
                    .Replace("%Assets%", (paths.AppRoot + "Assets").Path());
    return (bpf::io::File(path));
}

void AssetManager::Remove(const bpf::String &vpath)
{
    bool glob = vpath.EndsWith("*");
    bpf::String search;

    if (glob)
        search = vpath.Sub(0, vpath.Len() - 1);
    else
        search = vpath;
    for (auto &asset : _mountedAssets)
    {
        if (glob && asset.Value->VirtualPath().StartsWith(search))
        {
            _log.Info("Unloading asset '[]'...", asset.Value->VirtualPath());
            _mountedAssets[asset.Key] = Null;   //Force destruction of UniquePtr
            _mountedAssets.RemoveAt(asset.Key); //Inform HashMap that slot can be reclaimed
        }
        else if (!glob && asset.Value->VirtualPath() == search)
        {
            _log.Info("Unloading asset '[]'...", asset.Value->VirtualPath());
            _mountedAssets[asset.Key] = Null;   //Force destruction of UniquePtr
            _mountedAssets.RemoveAt(asset.Key); //Inform HashMap that slot can be reclaimed
            return;
        }
    }
}

void AssetManager::WaitForAllObjects()
{
    while (_thread.GetState() == bpf::system::Thread::RUNNING)
    {
        _thread.Join();
        while (Poll())
            ;
    }
}

bool AssetManager::AttemptSolveDependencies()
{
    if (_unresolved.Size() == 0)
        return (false);
    auto &entry = _unresolved.First();
    bool solved = true;
    for (auto &dep : entry.Builder->GetDependencies())
    {
        if (!_mountedAssets.HasKey(dep))
        {
            solved = false;
            break;
        }
    }
    if (!solved)
    {
        if (_thread.GetState() == bpf::system::Thread::FINISHED || _thread.GetState() == bpf::system::Thread::STOPPED)
        {
            _log.Error("Could not build asset '[]': some dependencies were not satisfied", entry.VPath);
            _unresolved.RemoveAt(_unresolved.begin());
            return (true);
        }
        return (true);
    }
    else
    {
        auto assetPtr = entry.Builder->Mount(*this, entry.VPath);
        if (assetPtr != Null)
            _mountedAssets.Add(bpf::Name(entry.VPath), std::move(assetPtr));
        _log.Info("Successfully loaded asset '[]'", entry.VPath);
        _unresolved.RemoveAt(_unresolved.begin());
        return (true);
    }
}

bool AssetManager::Poll(bpf::fsize maxMountable)
{
    while (maxMountable > 0)
    {
        AssetBuildThread::Entry entry;
        --maxMountable;
        if (!_thread.PollMountableEntry(entry))
            return (AttemptSolveDependencies());
        if (entry.Error != bpf::String::Empty)
        {
            _log.Error("Could not build asset '[]': an unhandled exception has occured", entry.VPath);
            _log.Error("        > []", entry.Error);
            continue;
        }
        const auto &expanded = entry.Builder->GetExpandedAssets();
        for (auto &tuple : expanded)
        {
            auto vpath = entry.VPath + '/' + tuple.Get<0>();
            auto url = tuple.Get<1>();
            Add(vpath, url);
        }
        bool solved = true;
        for (auto &dep : entry.Builder->GetDependencies())
        {
            if (!_mountedAssets.HasKey(dep))
            {
                _unresolved.Add(std::move(entry));
                solved = false;
                break;
            }
        }
        if (!solved)
            continue;
        auto assetPtr = entry.Builder->Mount(*this, entry.VPath);
        if (assetPtr != Null)
            _mountedAssets.Add(bpf::Name(entry.VPath), std::move(assetPtr));
        _log.Info("Successfully loaded asset '[]'", entry.VPath);
    }
    return (true);
}
