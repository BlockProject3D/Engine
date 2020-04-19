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
#include <Framework/IO/ByteBuf.hpp>
#include "Engine/Driver/ShaderProgramDescriptor.hpp"

namespace bp3d
{
    namespace driver
    {
        class BP3D_API IShader
        {
        public:
            struct Binding
            {
                /**
                 * The binding name
                 */
                bpf::String Name;

                /**
                 * The binding register number
                 */
                int Register;

                /**
                 * The binding type
                 * Only native binding types are listed: CONSTANT_BUFFER, TEXTURE, SAMPLER
                 */
                EBindingType Type;
            };
            struct PixelOutput
            {
                /**
                 * The output name
                 */
                bpf::String Name;

                /**
                 * The output register number
                 */
                int Register;

                /**
                 * Number of output channels (4 means RGBA, 1 means R only)
                 */
                bpf::uint8 Channels;
            };

            virtual ~IShader() {}
            virtual bpf::io::ByteBuf ToByteBuf() = 0;
            virtual const bpf::collection::ArrayList<Binding> &GetBindings() const noexcept = 0;
            virtual const bpf::collection::ArrayList<PixelOutput> &GetPixelOutputs() const noexcept = 0;
        };
    }
}
