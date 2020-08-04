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

#pragma once
#define BP_COMPAT_2_X
#include <Framework/Collection/HashMap.hpp>
#include <Framework/Memory/UniquePtr.hpp>
#include <Framework/Memory/ObjectPtr.hpp>
#include <Framework/TypeInfo.hpp>
#include <Framework/Log/Logger.hpp>
#include <Framework/System/Paths.hpp>
#include <Framework/Collection/List.hpp>
#include "Engine/IAssetProvider.hpp"
#include "Engine/Asset.hpp"
#include "Engine/AssetBuildThread.hpp"

//Asset url = <asset type>/<format>,(<root>/)<path/to/file.whatever>
//            [  Asset type info  ],[        Asset location        ]
//With root = %App% (application's root directory), %Cache% (application's cache directory), %Assets% (application's assets directory)
//When registering asset providers specify the format AssetManager.AddProvider<asset::MyAssetType>("mySuperAssetFormat");
//To load assets:
//      AssetManager.Add(<asset url>)
//      New assets are loaded asynchronously so add will append to a pending queue and no asset will immediatly be mounted
//Unloading assets
//      AssetManager.Remove(<virtual path>)
//      The virtual path can finish by * to request mass unloading of assets
//      This operation is synchronous and will reset all instances of ObjectPtr to Null
//      Attemoting to unload an asset set as default for a given type will result in this asset be ignored
//Polling
//      The calling application should call Poll on any asset manager to ensure the proper mounting of newly built assets
//      The function returns true when no more assets are pending mount
//Asset providers == OK
//      AssetManager.SetProvider<asset::MyAssetType>(const bpf::String &format, UniquePtr<IAssetProvider<asset::MyAssetType>> &&)
//      AssetManager.GetProvider<asset::MyAssetType>(const bpf::String &format)
//Defaults == OK
//      AssetManager.SetDefault<asset::MyAssetType>(<virtual path>)
//      AssetManager.GetDefault<asset::MyAssetType>()
//Getting assets == OK
//      AssetManager.Get<asset::MyAssetType>(<virtual path>)
//Injecting existing assets: == OK
//      AssetManager.Add<asset::MyAssetType>(UniquePtr<asset::MyAssetType> &&)

namespace bp3d
{
    class BP3D_API AssetManager
    {
    private:
        bpf::log::Logger _log;
        AssetBuildThread _thread;
        bpf::collection::HashMap<bpf::Name, bpf::memory::UniquePtr<bp3d::Asset>> _mountedAssets;
        bpf::collection::HashMap<bpf::String, bpf::memory::UniquePtr<IAssetProvider>> _providers;
        bpf::collection::HashMap<bpf::Name, bpf::Name> _defaults;
        bpf::collection::List<AssetBuildThread::Entry> _unresolved;

        bool AttemptSolveDependencies();
    public:
        inline AssetManager()
            : _log("AssetManager")
        {
        }
        inline ~AssetManager()
        {
            _thread.Join();
        }
        AssetManager(AssetManager &&other) = delete;
        AssetManager(const AssetManager &other) = delete;

        /**
         * Adds a new asset by url
         * Asset url = <asset type>/<format>,(<root>/)<path/to/file.whatever>
         *             [  Asset type info  ],[        Asset location        ]
         * @param url Asset url string
         */
        void Add(const bpf::String &vpath, const bpf::String &url);

        inline void AddLogHandler(bpf::memory::UniquePtr<bpf::log::ILogAdapter> &&ptr)
        {
            _log.AddHandler(std::move(ptr));
        }

        static bpf::io::File GetAssetPath(const bpf::system::Paths &paths, const bpf::String &location);

        template <typename T>
        inline void Add(bpf::memory::UniquePtr<Asset> &&ptr)
        {
            _mountedAssets.Add(ptr->HashCode(), ptr);
        }

        void Remove(const bpf::String &vpath);

        bool Poll(bpf::fsize maxMountable = 1);

        /**
         * Yields the main thread waiting for all pending asset objects to be mounted
         */
        void WaitForAllObjects();

        template <typename T>
        inline void SetProvider(const bpf::String &format, bpf::memory::UniquePtr<IAssetProvider> &&ptr)
        {
            _providers.Add(bpf::String(bpf::TypeName<T>()) + '/' + format, std::move(ptr));
        }

        inline bpf::memory::UniquePtr<IAssetProvider> &GetProvider(const bpf::String &format)
        {
            return (_providers[format]);
        }

        template <typename T>
        inline bpf::memory::ObjectPtr<T> Get(const bpf::Name &vpath) const noexcept
        {
            if (!_mountedAssets.HasKey(vpath) || _mountedAssets[vpath]->Type() != bpf::Name(bpf::TypeName<T>()))
                return (GetDefault<T>());
            return (static_cast<T *>(_mountedAssets[vpath].Raw()));
        }

        template <typename T>
        inline bpf::memory::ObjectPtr<T> GetDefault() const noexcept
        {
            auto tname = bpf::Name(bpf::TypeName<T>());

            if (!_defaults.HasKey(tname))
                return (Null); //We have no default registered
            return (static_cast<T *>(_mountedAssets[_defaults[tname]].Raw()));
        }

        template <typename T>
        inline void SetDefault(const bpf::Name &vpath)
        {
            auto tname = bpf::Name(bpf::TypeName<T>());

            if (!_mountedAssets.HasKey(vpath) || _mountedAssets[vpath]->Type() != tname)
                return;
            _defaults[tname] = vpath; //Will update or create entry in defaults map
        }

        AssetManager &operator=(const AssetManager &other) = delete;
        AssetManager &operator=(AssetManager &&other) = delete;
    };
}
