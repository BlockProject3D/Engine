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

#include "DX11ShaderCompiler.hpp"

using namespace dx11;

void DX11ShaderCompiler::SetMacro(const bpf::String &name, const bpf::String &value)
{
    _macroNames.Add(name);
    _macroValues.Add(value);
}

bp3d::driver::Resource DX11ShaderCompiler::Compile(const bpf::String &code, const bpf::String &name, const bp3d::driver::EShaderType type)
{
    auto lst = _macros;
    lst.Add(D3D_SHADER_MACRO{ Null, Null });
    auto arr = lst.ToArray();
    LPCSTR target = "vs_5_0";
    switch (type)
    {
    case bp3d::driver::EShaderType::DOMAIN:
        target = "ds_5_0";
        break;
    case bp3d::driver::EShaderType::GEOMETRY:
        target = "gs_5_0";
        break;
    case bp3d::driver::EShaderType::HULL:
        target = "hs_5_0";
        break;
    case bp3d::driver::EShaderType::PIXEL:
        target = "ps_5_0";
        break;
    case bp3d::driver::EShaderType::VERTEX:
        target = "vs_5_0";
        break;
    }
    UINT flags = 0;
    if (_flags & bp3d::driver::SHADER_COMPILE_DEBUG)
        flags |= D3DCOMPILE_DEBUG;
    if (_flags & bp3d::driver::SHADER_COMPILE_O0)
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
    if (_flags & bp3d::driver::SHADER_COMPILE_O1)
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
    if (_flags & bp3d::driver::SHADER_COMPILE_O2)
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
    if (_flags & bp3d::driver::SHADER_COMPILE_O3)
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    ID3DBlob *out;
    ID3DBlob *err;
    if (FAILED(D3DCompile(*code, code.Size(), *name, *arr, Null, "main", target, flags, 0, &out, &err)))
    {
        bpf::String str = reinterpret_cast<char *>(err->GetBufferPointer());
        throw bp3d::driver::ShaderException(str);
    }
    return (out);
}

void DX11ShaderCompiler::Link()
{
}

bpf::io::ByteBuf DX11ShaderCompiler::GetShaderByteCode(bp3d::driver::Resource resource)
{
    ID3DBlob *blob = reinterpret_cast<ID3DBlob *>(resource);
    bpf::io::ByteBuf buf(blob->GetBufferSize());
    buf.Write(blob->GetBufferPointer(), blob->GetBufferSize());
    return (buf);
}

void DX11ShaderCompiler::FreeHandle(bp3d::driver::Resource resource)
{
    ID3DBlob *blob = reinterpret_cast<ID3DBlob *>(resource);
    blob->Release();
}
