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
#include <Framework/Memory/Utility.hpp>
#undef min
#undef max
#include <Framework/Scalar.hpp>

using namespace dx11;

bool DX11Shader::IsAlphaNum(const bpf::fchar ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'));
}

bpf::String DX11Shader::ExtractName(const bpf::String &tname)
{
    bpf::String name;
    bpf::fisize lastId = tname.Len() - 1;

    while (!IsAlphaNum(tname[lastId]))
        --lastId;
    while (IsAlphaNum(tname[lastId]))
    {
        name = bpf::String(tname[lastId]) + name;
        --lastId;
    }
    return (name);
}

bpf::String DX11Shader::ExtractType(const bpf::String &tname)
{
    bpf::String name;
    bpf::fisize lastId = 0;

    while (!IsAlphaNum(tname[lastId]))
        ++lastId;
    while (IsAlphaNum(tname[lastId]))
    {
        name += tname[lastId];
        ++lastId;
    }
    return (name);
}

DX11Shader::DX11Shader(ID3DBlob *blob, const bpf::String &code, const bp3d::driver::EShaderType type)
    : _blob(blob)
{
    //This is my attempt at providing required HLSL reflection data in order to build a pipeline and a ShaderProgram
    auto strs = code.Explode(';');
    for (auto &str : strs)
    {
        auto arr = str.Explode(':');
        if (arr.Size() == 2) //Ok here we should get either semantics or registers
        {
            auto semantic = arr[1].Replace(" ", "").Replace("\t", "");
            if (semantic.StartsWith("register("))
            {
                auto reg = semantic.SubLen(9, 2);
                if (reg.Size() == 2)
                {
                    int regn = bpf::Int::Parse(reg.ByteAt(1));
                    auto name = ExtractName(arr[0]);
                    bp3d::driver::IShader::Binding bind;
                    //We know HLSL register name is in ascii
                    if (reg.ByteAt(0) == 't') //We got a texture binding
                        bind.Type = bp3d::driver::EBindingType::TEXTURE;
                    else if (reg.ByteAt(0) == 's') //We got a sampler binding
                        bind.Type = bp3d::driver::EBindingType::SAMPLER;
                    else if (reg.ByteAt(0) == 'b') //We got a constant buffer binding
                        bind.Type = bp3d::driver::EBindingType::CONSTANT_BUFFER;
                    bind.Name = name;
                    bind.Register = regn;
                    _bindings.Add(bind);
                }
            }
            if (type == bp3d::driver::EShaderType::PIXEL)
            {
                if (semantic.ToUpper().StartsWith("SV_TARGET"))
                {
                    auto reg = semantic.SubLen(9, 1);
                    int regn = bpf::Int::Parse(reg);
                    auto name = ExtractName(arr[0]);
                    auto tt = ExtractType(arr[0]);
                    bp3d::driver::IShader::PixelOutput pout;
                    pout.Channels = (tt == "float4" ? 4 : 1);
                    pout.Name = name;
                    pout.Register = regn;
                    _pouts.Add(pout);
                }
            }
        }
    }
}

DX11Shader::~DX11Shader()
{
    _blob->Release();
}

bpf::io::ByteBuf DX11Shader::ToByteBuf()
{
    bpf::io::ByteBuf buf(_blob->GetBufferSize());
    buf.Write(_blob->GetBufferPointer(), _blob->GetBufferSize());
    return (buf);
}

void DX11ShaderCompiler::SetMacro(const bpf::String &name, const bpf::String &value)
{
    _macroNames.Add(name);
    _macroValues.Add(value);
}

bpf::memory::UniquePtr<bp3d::driver::IShader> DX11ShaderCompiler::Compile(const bpf::String &code, const bpf::String &name, const bp3d::driver::EShaderType type)
{
    auto lst = _macros;
    lst.Add(D3D_SHADER_MACRO{Null, Null});
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
    return (bpf::memory::MakeUnique<DX11Shader>(out, code, type));
}

void DX11ShaderCompiler::Link()
{
}
