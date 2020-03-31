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
#include <Framework/System/Thread.hpp>
#include <Framework/System/Mutex.hpp>
#include <Framework/Collection/Queue.hpp>
#include "Engine/IAssetBuilder.hpp"

namespace bp3d
{
    class BP3D_API AssetBuildThread final : public bpf::system::Thread
    {
    public:
        struct Entry
        {
            bpf::String VPath;
            bpf::memory::UniquePtr<IAssetBuilder> Builder;
            bpf::collection::Queue<bpf::Tuple<bpf::String, bpf::String>> Cached;
            bpf::String Error;
        };

    private:
        bpf::collection::Queue<Entry> _pendingEntries;
        bpf::collection::Queue<Entry> _mountableEntries;
        bpf::system::Mutex _mutex;

    public:
        AssetBuildThread();

        void Add(const bpf::String &vpath, bpf::memory::UniquePtr<IAssetBuilder> &&ptr);
        bool PollMountableEntry(Entry &entry);
        void Run();
    };
}
