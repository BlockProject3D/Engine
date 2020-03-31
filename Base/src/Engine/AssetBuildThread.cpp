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

#include <Framework/System/ScopeLock.hpp>
#include "Engine/AssetBuildThread.hpp"

using namespace bp3d;

AssetBuildThread::AssetBuildThread()
    : bpf::system::Thread("AssetBuilder")
{
}

void AssetBuildThread::Add(const bpf::String &vpath, bpf::memory::UniquePtr<IAssetBuilder> &&ptr)
{
    auto lock = bpf::system::ScopeLock(_mutex);
    Entry entry;
    entry.VPath = vpath;
    entry.Builder = std::move(ptr);
    _pendingEntries.Push(std::move(entry));
}

bool AssetBuildThread::PollMountableEntry(Entry &entry)
{
    auto lock = bpf::system::ScopeLock(_mutex);
    if (_mountableEntries.Size() == 0)
        return (false);
    entry = _mountableEntries.Pop();
    return (true);
}

void AssetBuildThread::Run()
{
    _mutex.Lock();
    bpf::fsize size = _pendingEntries.Size();
    _mutex.Unlock();

    while (size > 0)
    {
        _mutex.Lock();
        Entry entry = _pendingEntries.Pop();
        _mutex.Unlock();
        bpf::collection::Queue<bpf::Tuple<bpf::String, bpf::String>> cache;
        try
        {
            entry.Builder->Build(cache);
            entry.Cached = std::move(cache);
            entry.Error = bpf::String::Empty;
            _mutex.Lock();
            _mountableEntries.Push(std::move(entry));
            _mutex.Unlock();
        }
        catch (const bpf::RuntimeException &ex)
        {
            entry.Error = bpf::String::Format("[]: []", ex.Type(), ex.Message());
            _mutex.Lock();
            _mountableEntries.Push(std::move(entry));
            _mutex.Unlock();
        }
    }
}
