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

#include <cstring>
#include "ShaderDecoder.hpp"
#include <Framework/System/Platform.hpp>
#include <Framework/RuntimeException.hpp>

using namespace gl40;

ShaderDecoder::ShaderDecoder(const void *data, const bpf::fsize size)
{
    BPGLSLHeader header = *reinterpret_cast<const BPGLSLHeader *>(data);
    if (bpf::system::Platform::GetEndianess() != bpf::system::EPlatformEndianess::PLATFORM_LITTLEENDIAN)
        bpf::system::Platform::ReverseBuffer(&header.Size, 4); //We know the size of a bpf::uint32 is always 32 bits or 4 bytes
    if (header.Signature[0] != 'B' || header.Signature[1] != 'P' || header.Signature[2] != 'G' || header.Signature[3] != 'L')
        throw bpf::RuntimeException("RenderEngine", "Invalid BPGLSL shader");
    if (header.Version != 0)
        throw bpf::RuntimeException("RenderEngine", "Unrecognized BPGLSL version");
    if (sizeof(BPGLSLHeader) + header.Size > size)
        throw bpf::RuntimeException("RenderEngine", "Invalid or corrupted BPGLSL shader");
    const char *shader = reinterpret_cast<const char *>(data) + sizeof(BPGLSLHeader);
    _shaderCode = shader;
    _metadata = shader + header.Size;
    _size = header.Size;
    _ucount = header.UniformCount;
}

bool ShaderDecoder::GetNextUniform(BPGLSLUniform &uniform) noexcept
{
    if (_ucount == 0)
        return (false);
    uniform.Register = (bpf::uint8)_metadata[0];
    uniform.Name = _metadata + 1;
    _metadata += std::strlen(_metadata) + 2;
    --_ucount;
}