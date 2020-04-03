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
#include <Framework/Types.hpp>

namespace bp3d
{
    namespace driver
    {
        enum class BP3D_API ETextureFormat
        {
            RGB_UINT_8,
            RGBA_UINT_8,
            R_UINT_8,
            R_FLOAT_32
        };

        enum class BP3D_API ETextureCompression
        {
            NONE,
            BC1_OR_DXT1,
            BC2_OR_DXT3,
            BC3_OR_DXT5
        };

        struct BP3D_API TextureDescriptor
        {
            ETextureFormat Format;
            ETextureCompression Compression;
            bpf::fint MipMaps;
            bpf::fsize Width;
            bpf::fsize Height;
            bpf::uint32 SampleLevel; //Multisampling level x2, x4, x8...
            bpf::uint32 QualityLevel; //Quality level for multisampling; cannot be greater than MaxImageQuality
            void *Data;
        };
    }
}
