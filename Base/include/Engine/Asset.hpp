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
#include <Framework/Memory/Object.hpp>
#include <Framework/String.hpp>
#include <Framework/Name.hpp>
#include <Framework/TypeInfo.hpp>

namespace bp3d
{
    class BP3D_API Asset : public bpf::memory::Object
    {
    private:
        bpf::Name _type;
        bpf::Name _vPathHash;
        bpf::String _vPathStr;

    public:
        inline Asset(const bpf::Name &type, const bpf::String &vpath)
            : _type(type)
            , _vPathHash(bpf::Name(vpath))
            , _vPathStr(vpath)
        {
        }

        inline Asset()
            : _type("bp3d::Asset")
            , _vPathHash("Invalid")
            , _vPathStr("Invalid")
        {
        }

        inline const bpf::String &VirtualPath() const noexcept
        {
            return (_vPathStr);
        }

        inline bpf::Name HashCode() const noexcept
        {
            return (_vPathHash);
        }

        inline bpf::Name Type() const noexcept
        {
            return (_type);
        }
    };
}

BP_DEFINE_TYPENAME(bp3d::Asset);
