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
#undef DOMAIN
#include <Framework/Types.hpp>
#include <Framework/Collection/ArrayList.hpp>

namespace bp3d
{
    namespace driver
    {
        constexpr int LOCK_VERTEX_STAGE = 0x1;
        constexpr int LOCK_HULL_STAGE = 0x2;
        constexpr int LOCK_DOMAIN_STAGE = 0x4;
        constexpr int LOCK_GEOMETRY_STAGE = 0x8;
        constexpr int LOCK_PIXEL_STAGE = 0x10;
        constexpr int LOCK_ALL_STAGE = LOCK_VERTEX_STAGE | LOCK_HULL_STAGE | LOCK_DOMAIN_STAGE | LOCK_GEOMETRY_STAGE | LOCK_PIXEL_STAGE;

        enum class BP3D_API EShaderType
        {
            /**
             * Vertex shader stage
             */
            VERTEX,

            /**
             * Hull shader stage also called Tesselation Control stage in OpenGL
             */
            HULL,

            /**
             * Geometry shader stage
             */
            GEOMETRY,

            /**
             * Domain shader stage also called Tesselation Evaluation stage in OpenGL
             */
            DOMAIN,

            /**
             * Pixel shader stage also called Fragment shader stage in OpenGL
             */
            PIXEL
        };

        enum class BP3D_API EBindingType
        {
            TEXTURE,
            SAMPLER,
            CONSTANT_BUFFER,
            FIXED_CONSTANT_BUFFER
        };

        struct BP3D_API ShaderDescriptor
        {
            EShaderType Type;
            bpf::fsize Size;
            void *Data;
        };

        struct BP3D_API ShaderBindingDescriptor
        {
            int Register;
            int StageFlags;
            EBindingType Type;
        };

        struct BP3D_API ShaderProgramDescriptor
        {
            bpf::collection::ArrayList<ShaderDescriptor> Shaders;
            bpf::collection::ArrayList<ShaderBindingDescriptor> Bindings;
        };
    }
}
