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

#include "GL40ShaderCompiler.hpp"
#include <Framework/Scalar.hpp>

using namespace gl40;

bool GL40Shader::IsAlphaNum(const bpf::fchar ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'));
}

void GL40Shader::ExtractTypeName(const bpf::String &tname, bpf::String &subType, bpf::String &name, bpf::String &type)
{
    bpf::fisize lastId = 0;

    name = "";
    type = "";
    subType = "";
    while (!IsAlphaNum(tname[lastId]))
        ++lastId;
    while (IsAlphaNum(tname[lastId]))
    {
        subType += tname[lastId];
        ++lastId;
    }
    while (!IsAlphaNum(tname[lastId]))
        ++lastId;
    while (IsAlphaNum(tname[lastId]))
    {
        type += tname[lastId];
        ++lastId;
    }
    if (lastId < tname.Len())
    {
        while (!IsAlphaNum(tname[lastId]))
            ++lastId;
        while (IsAlphaNum(tname[lastId]))
        {
            name += tname[lastId];
            ++lastId;
        }
    }
}

GL40Shader::GL40Shader(const bpf::String &code, const bp3d::driver::EShaderType stype)
{
    bpf::String fixedCode = "";
    auto arr = code.Explode(';');
    for (auto &statement : arr)
    {
        if (statement.Contains("layout"))
        {
            auto startId = statement.IndexOf("layout");
            auto endId = statement.IndexOf(')');
            auto nametype = statement.Sub(endId + 1);
            auto layout = statement.Sub(startId + 7, endId);
            layout = layout.Sub(layout.IndexOf('(') + 1);
            auto fixedStatement = statement.Sub(0, startId) + nametype;
            fixedCode += fixedStatement + ';';
            layout = layout.Replace(" ", "").Replace("\t", "");
            auto attributes = layout.Explode(',');
            for (auto &attribute : attributes)
            {
                auto attributedata = attribute.Explode('=');
                if (attributedata.Size() == 2) //Treat both location and binding as the same thing
                {
                    int regn;
                    //Use TryParse as we cannot check if shader is fine at least yet
                    if (bpf::Int::TryParse(attributedata[1], regn))
                    {
                        bpf::String name;
                        bpf::String type;
                        bpf::String subType;
                        ExtractTypeName(nametype, subType, name, type);
                        if (stype == bp3d::driver::EShaderType::PIXEL && subType == "out")
                        { //Ok we got a render target output
                            bp3d::driver::IShader::PixelOutput pout;
                            if (type == "vec4")
                                pout.Channels = 4;
                            else
                                pout.Channels = 1;
                            pout.Name = name;
                            pout.Register = regn;
                            _pouts.Add(pout);
                        }
                        else
                        {
                            if (type == "sampler2D")
                            {
                                bp3d::driver::IShader::Binding bind;
                                bind.Name = name;
                                bind.Type = bp3d::driver::EBindingType::TEXTURE; //In GLSL Texture bindings are also sampler bindings
                                bind.Register = regn;
                                _bindings.Add(bind);
                            }
                            else
                            { //If it's not a texture it must be a uniform block / constant buffer
                                bp3d::driver::IShader::Binding bind;
                                bind.Name = type;
                                bind.Register = regn;
                                bind.Type = bp3d::driver::EBindingType::CONSTANT_BUFFER;
                                _bindings.Add(bind);
                            }
                        }
                        break;
                    }
                }
            }
        }
        else
            fixedCode += statement + ';';
    }
}

bpf::io::ByteBuf GL40Shader::ToByteBuf()
{
    bpf::io::ByteBuf buf(128);
    //Algorithm:
    // 1) Calc size of shader code
    // 2) Foreach bindings calc size of name + size of int
    // 3) Alloc ByteBuf
    // 4) Write data
    return (buf);
}

void GL40ShaderCompiler::SetMacro(const bpf::String &name, const bpf::String &value)
{
    _macroNames.Add(name);
    _macroValues.Add(value);
}

void GL40ShaderCompiler::SetCompileFlags(const bpf::fint flags)
{
}
