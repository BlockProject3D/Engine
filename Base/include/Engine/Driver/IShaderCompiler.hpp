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
#include <Framework/IO/ByteBuf.hpp>
#include <Framework/String.hpp>
#include <Framework/RuntimeException.hpp>
#include "Engine/Driver/ShaderProgramDescriptor.hpp"
#include "Engine/Driver/Resource.hpp"

namespace bp3d
{
    namespace driver
    {
        constexpr bpf::fint SHADER_COMPILE_DEBUG = 0x1;
        constexpr bpf::fint SHADER_COMPILE_O0 = 0x2;
        constexpr bpf::fint SHADER_COMPILE_O1 = 0x4;
        constexpr bpf::fint SHADER_COMPILE_O2 = 0x8;
        constexpr bpf::fint SHADER_COMPILE_O3 = 0x10;

        class BP3D_API ShaderException : public bpf::RuntimeException
        {
        public:
            inline ShaderException(const bpf::String &message) noexcept
                : bpf::RuntimeException("Shader", message)
            {
            }
        };

        class BP3D_API IShaderCompiler
        {
        public:
            virtual ~IShaderCompiler() {}
            virtual void SetMacro(const bpf::String &name, const bpf::String &value) = 0;
            virtual void SetCompileFlags(const bpf::fint flags) = 0;

            /**
             * Compiles a shader
             * WARNING: The shader compiler does not automatically free compiled shaders
             * @param code the shader code
             * @param name a name for the shader when printing errors
             * @param type the type of the shader to compile
             * @throw ShaderException in case of a compile error
             * @return an opaque handle to the shader that has just been compiled
             */
            virtual Resource Compile(const bpf::String &code, const bpf::String &name, const EShaderType type) = 0;

            /**
             * Attempts to link all previously compiled shaders into a shader program
             * @throw ShaderException in case of a link error
             */
            virtual void Link() = 0;

            /**
             * Returns a bytebuf containing shader byte code for the specified opaque handle
             */
            virtual bpf::io::ByteBuf GetShaderByteCode(Resource resource) = 0;

            /**
             * Free the given opaque handle
             * @param the opaque handle to free
             */
            virtual void FreeHandle(Resource resource) = 0;
        };
    }
}
