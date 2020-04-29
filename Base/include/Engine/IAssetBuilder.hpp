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
#include <Framework/String.hpp>
#include <Framework/Tuple.hpp>
#include <Framework/Collection/List.hpp>
#include <Framework/Memory/UniquePtr.hpp>
#include "Engine/Asset.hpp"

namespace bp3d
{
    //Forward declare the AssetManager to avoid circular dependency between AssetManager and IAssetBuilder
    class BP3D_API AssetManager;

    class BP3D_API IAssetBuilder
    {
    public:
        virtual ~IAssetBuilder() {}

        /**
         * Builds this asset asynchronously
         * This function may not be called on the main thread so beware of race conditions
         * It is unsafe to call any rendering method or driver resource allocations in this function
         * This method typically runs file IO and pre-calculations in order to prepare the data for the Mount method
         */
        virtual void Build() = 0;

        /**
         * Returns a list of assets to be loaded as a result of the expansion of this asset
         * This method is typically used for packages/archives and/or other similar types of assets
         */
        virtual const bpf::collection::List<bpf::Tuple<bpf::String, bpf::String>> &GetExpandedAssets() const noexcept = 0;

        /**
         * Returns a list of dependencies that must be mounted before this asset can be mounted
         */
        virtual const bpf::collection::List<bpf::Name> &GetDependencies() const noexcept = 0;

        /**
         * Callback after a successfull call to build on the main thread
         * Use this function to allocate new driver resources
         * @param assets The instance of the AssetManager responsible for this IAssetBuilder, you can use this instance to get your dependencies
         * @param vpath The virtual path of the Asset to be mounted
         * @return The new mounted Asset or Null if no asset is required to be mounted for this asset
         */
        virtual bpf::memory::UniquePtr<Asset> Mount(AssetManager &assets, const bpf::String &vpath) = 0;
    };
}
