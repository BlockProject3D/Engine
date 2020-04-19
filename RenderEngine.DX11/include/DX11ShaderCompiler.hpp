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
#include <Engine/Driver/IShaderCompiler.hpp>
#include <d3dcompiler.h>

namespace dx11
{
    class DX11Shader final : public bp3d::driver::IShader
    {
    private:
        ID3DBlob *_blob;
        bpf::collection::ArrayList<bp3d::driver::IShader::Binding> _bindings;
        bpf::collection::ArrayList<bp3d::driver::IShader::PixelOutput> _pouts;

        bpf::String ExtractName(const bpf::String &tname);
        bpf::String ExtractType(const bpf::String &tname);
        bool IsAlphaNum(const bpf::fchar ch);

    public:
        DX11Shader(ID3DBlob *blob, const bpf::String &code, const bp3d::driver::EShaderType type);
        ~DX11Shader();
        bpf::io::ByteBuf ToByteBuf();
        inline const bpf::collection::ArrayList<bp3d::driver::IShader::Binding> &GetBindings() const noexcept
        {
            return (_bindings);
        }
        inline const bpf::collection::ArrayList<bp3d::driver::IShader::PixelOutput> &GetPixelOutputs() const noexcept
        {
            return (_pouts);
        }
    };

    class DX11ShaderCompiler final : public bp3d::driver::IShaderCompiler
    {
    private:
        bpf::fint _flags;
        bpf::collection::ArrayList<D3D_SHADER_MACRO> _macros;
        bpf::collection::ArrayList<bpf::String> _macroNames;
        bpf::collection::ArrayList<bpf::String> _macroValues;

    public:
        inline DX11ShaderCompiler()
            : _flags(0)
        {
        }

        void SetMacro(const bpf::String &name, const bpf::String &value);
        inline void SetCompileFlags(const bpf::fint flags)
        {
            _flags = flags;
        }
        bpf::memory::UniquePtr<bp3d::driver::IShader> Compile(const bpf::String &code, const bpf::String &name, const bp3d::driver::EShaderType type);
        void Link();
    };
}