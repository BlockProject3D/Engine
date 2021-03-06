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

#define BP_COMPAT_2_X

#include <Framework/RuntimeException.hpp>
#include "DX11ResourceAllocator.hpp"
#include "DX11Resources.hpp"

using namespace dx11;

ID3DBlob *DX11ResourceAllocator::CompileDummyShader(const bp3d::driver::VertexFormatDescriptor &descriptor)
{
    bpf::String str = bpf::String::Empty;
    for (auto &comp : descriptor.Components)
    {
        switch (comp.Type)
        {
        case bp3d::driver::EVertexComponentType::FLOAT:
            str += "float ";
            break;
        case bp3d::driver::EVertexComponentType::INT:
            str += "int ";
            break;
        case bp3d::driver::EVertexComponentType::UINT:
            str += "uint ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_2:
            str += "float2 ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_3:
            str += "float3 ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_4:
            str += "float4 ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_2:
            str += "int2 ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_3:
            str += "int3 ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_4:
            str += "int4 ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_2:
            str += "uint2 ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_3:
            str += "uint3 ";
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_4:
            str += "uint4 ";
            break;
        }
        str += comp.Name + ":" + comp.Name.ToUpper() + ";";
    }
    bpf::String inputStruct = bpf::String("struct ") + descriptor.Name + '{' +  str + "};";
    bpf::String outputStruct = bpf::String("struct DummyOutput{float4 Dummy:SV_POSITION;") + str + "};";
    bpf::String mainFunc = bpf::String("DummyOutput main(") + descriptor.Name + " input){DummyOutput output;output.Dummy=float4(0,0,0,0);";
    for (auto &comp : descriptor.Components)
        mainFunc += bpf::String("output.") + comp.Name + '=' + "input." + comp.Name + ';';
    mainFunc += "return (output);}";
    bpf::String dummyShaderCode = inputStruct + outputStruct + mainFunc;
    D3D_SHADER_MACRO dummy[] = { Null, Null };
    ID3DBlob *blob;
    ID3DBlob *useless;
    if (FAILED(D3DCompile(*dummyShaderCode, dummyShaderCode.Size(), Null, dummy, Null, "main", "vs_5_0", D3DCOMPILE_SKIP_VALIDATION, 0, &blob, &useless)))
        throw bpf::RuntimeException("RenderEngine", "Failed to compile vertex format");
    return (blob);
}

void DX11ResourceAllocator::SetupTextureFormat(const bp3d::driver::TextureDescriptor &descriptor, D3D11_TEXTURE2D_DESC &desc, UINT &sysmempitch, UINT &sysmemslicepitch, D3D11_SHADER_RESOURCE_VIEW_DESC &shaderDesc)
{
    switch (descriptor.Format)
    {
    case bp3d::driver::ETextureFormat::RGBA_UINT_8:
        sysmempitch = (UINT)descriptor.Width * 4;
        sysmemslicepitch = (UINT)descriptor.Width * (UINT)descriptor.Height * 4;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        shaderDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    case bp3d::driver::ETextureFormat::R_UINT_8:
        sysmempitch = (UINT)descriptor.Width;
        sysmemslicepitch = (UINT)descriptor.Width * (UINT)descriptor.Height;
        desc.Format = DXGI_FORMAT_R8_UNORM;
        shaderDesc.Format = DXGI_FORMAT_R8_UNORM;
        break;
    case bp3d::driver::ETextureFormat::R_FLOAT_32:
        sysmempitch = (UINT)descriptor.Width * 4;
        sysmemslicepitch = (UINT)descriptor.Width * (UINT)descriptor.Height * 4;
        desc.Format = DXGI_FORMAT_R32_FLOAT;
        shaderDesc.Format = DXGI_FORMAT_R32_FLOAT;
        break;
    }
    if (descriptor.Compression != bp3d::driver::ETextureCompression::NONE)
    {
        switch (descriptor.Compression)
        {
        case bp3d::driver::ETextureCompression::BC1_OR_DXT1:
            desc.Format = DXGI_FORMAT_BC1_UNORM;
            break;
        case bp3d::driver::ETextureCompression::BC2_OR_DXT3:
            desc.Format = DXGI_FORMAT_BC2_UNORM;
            break;
        case bp3d::driver::ETextureCompression::BC3_OR_DXT5:
            desc.Format = DXGI_FORMAT_BC3_UNORM;
            break;
        }
    }
}

DX11ResourceAllocator::DX11ResourceAllocator(ID3D11Device *dev, ID3D11DeviceContext *devContext, const bp3d::driver::RenderProperties &rprops)
    : _device(dev)
    , _deviceContext(devContext)
{
    for (UINT i = 0; i != 12; ++i)
        _states[i] = Null;
    for (UINT i = 0; i != 4; ++i)
        _depthStates[i] = Null;
    _baseDesc.FillMode = D3D11_FILL_SOLID;
    _baseDesc.CullMode = D3D11_CULL_NONE;
    _baseDesc.FrontCounterClockwise = TRUE;
    _baseDesc.DepthBias = 0;
    _baseDesc.DepthBiasClamp = 0;
    _baseDesc.DepthClipEnable = TRUE;
    _baseDesc.ScissorEnable = FALSE;
    _baseDesc.MultisampleEnable = rprops.Multisampling ? TRUE : FALSE;
    _baseDesc.AntialiasedLineEnable = rprops.Antialiasing ? TRUE : FALSE;
}

DX11ResourceAllocator::~DX11ResourceAllocator()
{
    for (UINT i = 0; i != 12; ++i)
    {
        if (_states[i] != Null)
            _states[i]->Release();
    }
    for (UINT i = 0; i != 4; ++i)
    {
        if (_depthStates[i] != Null)
            _depthStates[i]->Release();
    }
}

ID3D11RasterizerState *DX11ResourceAllocator::GetRasterizerState(const bp3d::driver::PipelineDescriptor &descriptor)
{
    D3D11_CULL_MODE cullMode = D3D11_CULL_NONE;
    D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID;
    switch (descriptor.CullingMode)
    {
    case bp3d::driver::ECullingMode::BACK_FACE:
        cullMode = D3D11_CULL_BACK;
        break;
    case bp3d::driver::ECullingMode::FRONT_FACE:
        cullMode = D3D11_CULL_FRONT;
        break;
    case bp3d::driver::ECullingMode::DISABLED:
        cullMode = D3D11_CULL_NONE;
        break;
    }
    switch (descriptor.RenderMode)
    {
    case bp3d::driver::ERenderMode::TRIANGLES:
        fillMode = D3D11_FILL_SOLID;
        break;
    case bp3d::driver::ERenderMode::WIREFRAME:
        fillMode = D3D11_FILL_WIREFRAME;
        break;
    }
    UINT state = DX11_RASTERIZER_INDEX(cullMode, descriptor.ScissorEnable ? TRUE : FALSE, fillMode);
    if (_states[state] == Null)
    {
        D3D11_RASTERIZER_DESC desc = _baseDesc;
        desc.CullMode = cullMode;
        desc.FillMode = fillMode;
        desc.ScissorEnable = descriptor.ScissorEnable ? TRUE : FALSE;
        if (FAILED(_device->CreateRasterizerState(&desc, &_states[state])))
            throw bpf::RuntimeException("RenderEngine", "CreateRasterizerState failed");
    }
    return (_states[state]);
}

ID3D11DepthStencilState *DX11ResourceAllocator::GetDepthState(const bp3d::driver::PipelineDescriptor &descriptor)
{
    UINT state = DX11_DEPTHSTATE_INDEX(descriptor.DepthEnable, descriptor.DepthWriteEnable);
    if (_depthStates[state] == Null)
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
        depthStencilDesc.DepthEnable = descriptor.DepthEnable;
        depthStencilDesc.DepthWriteMask = descriptor.DepthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depthStencilDesc.StencilEnable = false;
        depthStencilDesc.StencilReadMask = 0xFF;
        depthStencilDesc.StencilWriteMask = 0xFF;
        depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        if (FAILED(_device->CreateDepthStencilState(&depthStencilDesc, &_depthStates[state])))
            throw bpf::RuntimeException("RenderEngine", "CreateDepthStencilState failed");
    }
    return (_depthStates[state]);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocTexture2D(const bp3d::driver::EBufferType type, const bp3d::driver::TextureDescriptor &descriptor)
{
    D3D11_SUBRESOURCE_DATA data;
    D3D11_TEXTURE2D_DESC desc;
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
    ID3D11Texture2D *texture;
    ID3D11ShaderResourceView *view;
    shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    data.pSysMem = descriptor.Data;
    desc.Width = (UINT)descriptor.Width;
    desc.Height = (UINT)descriptor.Height;
    desc.MipLevels = (UINT)descriptor.MipMaps;
    desc.ArraySize = 1;
    SetupTextureFormat(descriptor, desc, data.SysMemPitch, data.SysMemSlicePitch, shaderDesc);
    desc.SampleDesc.Count = descriptor.SampleLevel;
    desc.SampleDesc.Quality = descriptor.QualityLevel;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        break;
    case bp3d::driver::EBufferType::STATIC:
        desc.Usage = D3D11_USAGE_IMMUTABLE; //A texture 2d is never updated by GPU except if it is part of a RenderTarget
        desc.CPUAccessFlags = 0;
        break;
    }
    if (descriptor.MipMaps > 1)
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }
    else
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = 0;
    }
    if (FAILED(_device->CreateTexture2D(&desc, &data, &texture)))
        throw bpf::RuntimeException("RenderEngine", "CreateTexture2D failed");
    shaderDesc.Format = desc.Format;
    shaderDesc.Texture2D.MostDetailedMip = 0;
    shaderDesc.Texture2D.MipLevels = desc.MipLevels;
    if (FAILED(_device->CreateShaderResourceView(texture, &shaderDesc, &view)))
    {
        texture->Release();
        throw bpf::RuntimeException("RenderEngine", "CreateShaderResourceView failed");
    }
    if (descriptor.MipMaps > 1)
        _deviceContext->GenerateMips(view);
    Texture2D *tex = static_cast<Texture2D *>(bpf::memory::Memory::Malloc(sizeof(Texture2D)));
    tex->Texture = texture;
    tex->View = view;
    return (tex);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocTexture2DArray(const bp3d::driver::EBufferType type, const bp3d::driver::TextureDescriptor &descriptor, const bpf::fsize layers)
{
    UINT sysmempitch = 0;
    UINT sysmemslicepitch = 0;
    D3D11_TEXTURE2D_DESC desc;
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
    ID3D11Texture2D *texture;
    ID3D11ShaderResourceView *view;
    shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    desc.Width = (UINT)descriptor.Width;
    desc.Height = (UINT)descriptor.Height;
    desc.MipLevels = (UINT)descriptor.MipMaps;
    desc.ArraySize = (UINT)layers;
    SetupTextureFormat(descriptor, desc, sysmempitch, sysmemslicepitch, shaderDesc);
    desc.SampleDesc.Count = descriptor.SampleLevel;
    desc.SampleDesc.Quality = descriptor.QualityLevel;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        break;
    case bp3d::driver::EBufferType::STATIC:
        desc.Usage = D3D11_USAGE_IMMUTABLE; //A texture 2d is never updated by GPU except if it is part of a RenderTarget
        desc.CPUAccessFlags = 0;
        break;
    }
    if (descriptor.MipMaps > 1)
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }
    else
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = 0;
    }
    bpf::collection::Array<D3D11_SUBRESOURCE_DATA> resoucres(layers);
    bpf::uint8 *ptr = reinterpret_cast<bpf::uint8 *>(descriptor.Data);
    for (bpf::fsize i = 0; i != layers; ++i)
    {
        resoucres[i].SysMemPitch = sysmempitch;
        resoucres[i].SysMemSlicePitch = sysmemslicepitch;
        resoucres[i].pSysMem = ptr + (sysmemslicepitch * i);
    }
    if (FAILED(_device->CreateTexture2D(&desc, *resoucres, &texture)))
        throw bpf::RuntimeException("RenderEngine", "CreateTexture2D failed");
    shaderDesc.Texture2DArray.ArraySize = (UINT)layers;
    shaderDesc.Texture2DArray.MostDetailedMip = 0;
    shaderDesc.Texture2DArray.MipLevels = desc.MipLevels;
    shaderDesc.Texture2DArray.FirstArraySlice = 0;
    shaderDesc.Format = desc.Format;
    if (FAILED(_device->CreateShaderResourceView(texture, &shaderDesc, &view)))
    {
        texture->Release();
        throw bpf::RuntimeException("RenderEngine", "CreateShaderResourceView failed");
    }
    if (descriptor.MipMaps > 1)
        _deviceContext->GenerateMips(view);
    Texture2D *tex = static_cast<Texture2D *>(bpf::memory::Memory::Malloc(sizeof(Texture2D)));
    tex->Texture = texture;
    tex->View = view;
    return (tex);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocTextureCube(const bp3d::driver::EBufferType type, const bp3d::driver::TextureDescriptor &descriptor)
{
    UINT sysmempitch = 0;
    UINT sysmemslicepitch = 0;
    D3D11_TEXTURE2D_DESC desc;
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
    ID3D11Texture2D *texture;
    ID3D11ShaderResourceView *view;
    shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    desc.Width = (UINT)descriptor.Width;
    desc.Height = (UINT)descriptor.Height;
    desc.MipLevels = (UINT)descriptor.MipMaps;
    desc.ArraySize = 6;
    SetupTextureFormat(descriptor, desc, sysmempitch, sysmemslicepitch, shaderDesc);
    desc.SampleDesc.Count = descriptor.SampleLevel;
    desc.SampleDesc.Quality = descriptor.QualityLevel;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        break;
    case bp3d::driver::EBufferType::STATIC:
        desc.Usage = D3D11_USAGE_IMMUTABLE; //A texture 2d is never updated by GPU except if it is part of a RenderTarget
        desc.CPUAccessFlags = 0;
        break;
    }
    if (descriptor.MipMaps > 1)
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
    }
    else
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    }
    D3D11_SUBRESOURCE_DATA textures[6];
    bpf::uint8 *ptr = reinterpret_cast<bpf::uint8 *>(descriptor.Data);
    for (bpf::fsize i = 0; i != 6; ++i)
    {
        textures[i].SysMemPitch = sysmempitch;
        textures[i].SysMemSlicePitch = sysmemslicepitch;
        textures[i].pSysMem = ptr + (sysmemslicepitch * i);
    }
    if (FAILED(_device->CreateTexture2D(&desc, textures, &texture)))
        throw bpf::RuntimeException("RenderEngine", "CreateTexture2D failed");
    shaderDesc.Format = desc.Format;
    shaderDesc.TextureCube.MostDetailedMip = 0;
    shaderDesc.TextureCube.MipLevels = desc.MipLevels;
    if (FAILED(_device->CreateShaderResourceView(texture, &shaderDesc, &view)))
    {
        texture->Release();
        throw bpf::RuntimeException("RenderEngine", "CreateShaderResourceView failed");
    }
    if (descriptor.MipMaps > 1)
        _deviceContext->GenerateMips(view);
    Texture2D *tex = static_cast<Texture2D *>(bpf::memory::Memory::Malloc(sizeof(Texture2D)));
    tex->Texture = texture;
    tex->View = view;
    return (tex);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocSampler(const bp3d::driver::SamplerDescriptor &descriptor)
{
    ID3D11SamplerState *state;
    D3D11_SAMPLER_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_SAMPLER_DESC));
    switch (descriptor.AddressModeU)
    {
    case bp3d::driver::ETextureAddressing::CLAMP_TO_EDGE:
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        break;
    case bp3d::driver::ETextureAddressing::MIRRORED_REPEAT:
        desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
        break;
    case bp3d::driver::ETextureAddressing::REPEAT:
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        break;
    }
    switch (descriptor.AddressModeV)
    {
    case bp3d::driver::ETextureAddressing::CLAMP_TO_EDGE:
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        break;
    case bp3d::driver::ETextureAddressing::MIRRORED_REPEAT:
        desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
        break;
    case bp3d::driver::ETextureAddressing::REPEAT:
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        break;
    }
    switch (descriptor.AddressModeW)
    {
    case bp3d::driver::ETextureAddressing::CLAMP_TO_EDGE:
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        break;
    case bp3d::driver::ETextureAddressing::MIRRORED_REPEAT:
        desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
        break;
    case bp3d::driver::ETextureAddressing::REPEAT:
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        break;
    }
    switch (descriptor.FilterFunc)
    {
    case bp3d::driver::ETextureFiltering::MIN_MAG_LINEAR_MIPMAP_LINEAR:
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    case bp3d::driver::ETextureFiltering::MIN_MAG_LINEAR_MIPMAP_POINT:
        desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        break;
    case bp3d::driver::ETextureFiltering::MIN_MAG_POINT_MIPMAP_LINEAR:
        desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        break;
    case bp3d::driver::ETextureFiltering::MIN_MAG_POINT_MIPMAP_POINT:
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        break;
    case bp3d::driver::ETextureFiltering::ANISOTROPIC:
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        break;
    }
    desc.MaxAnisotropy = descriptor.AnisotropicLevel;
    if (FAILED(_device->CreateSamplerState(&desc, &state)))
        throw bpf::RuntimeException("RenderEngine", "CreateSamplerState failed");
    return (state);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocRenderTargetComponent(const bp3d::driver::RenderTargetComponentDescriptor &descriptor)
{
    D3D11_SUBRESOURCE_DATA data;
    D3D11_TEXTURE2D_DESC desc;
    ID3D11Texture2D *texture;
    ID3D11ShaderResourceView *view = Null;
    data.pSysMem = descriptor.Texture.Data;
    desc.Width = (UINT)descriptor.Texture.Width;
    desc.Height = (UINT)descriptor.Texture.Height;
    desc.MipLevels = (UINT)descriptor.Texture.MipMaps;
    desc.ArraySize = 1;
    switch (descriptor.Texture.Format)
    {
    case bp3d::driver::ETextureFormat::RGBA_UINT_8:
        data.SysMemPitch = (UINT)descriptor.Texture.Width * 4;
        data.SysMemSlicePitch = (UINT)descriptor.Texture.Width * (UINT)descriptor.Texture.Height * 4;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    case bp3d::driver::ETextureFormat::R_UINT_8:
        data.SysMemPitch = (UINT)descriptor.Texture.Width;
        data.SysMemSlicePitch = (UINT)descriptor.Texture.Width * (UINT)descriptor.Texture.Height;
        desc.Format = DXGI_FORMAT_R8_UNORM;
        break;
    case bp3d::driver::ETextureFormat::R_FLOAT_32:
        data.SysMemPitch = (UINT)descriptor.Texture.Width * 4;
        data.SysMemSlicePitch = (UINT)descriptor.Texture.Width * (UINT)descriptor.Texture.Height * 4;
        desc.Format = DXGI_FORMAT_R32_FLOAT;
        break;
    }
    desc.SampleDesc.Count = descriptor.Texture.SampleLevel;
    desc.SampleDesc.Quality = descriptor.Texture.QualityLevel;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    if (descriptor.Renderable)
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    else
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (descriptor.Texture.MipMaps > 1)
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    else
        desc.MiscFlags = 0;
    if (FAILED(_device->CreateTexture2D(&desc, &data, &texture)))
        throw bpf::RuntimeException("RenderEngine", "CreateTexture2D failed");
    if (descriptor.Renderable)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
        shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderDesc.Format = desc.Format;
        shaderDesc.Texture2D.MostDetailedMip = 0;
        shaderDesc.Texture2D.MipLevels = desc.MipLevels;
        if (FAILED(_device->CreateShaderResourceView(texture, &shaderDesc, &view)))
        {
            texture->Release();
            throw bpf::RuntimeException("RenderEngine", "CreateShaderResourceView failed");
        }
        if (descriptor.Texture.MipMaps > 1)
            _deviceContext->GenerateMips(view);
    }
    Texture2D *tex = static_cast<Texture2D *>(bpf::memory::Memory::Malloc(sizeof(Texture2D)));
    tex->Format = desc.Format;
    tex->Texture = texture;
    if (descriptor.Renderable)
        tex->View = view;
    else
        tex->View = Null;
    return (tex);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocDepthBuffer(const bpf::fsize width, const bpf::fsize height, const bp3d::driver::EDepthBufferFormat format)
{
    ID3D11Texture2D *tex;
    ID3D11DepthStencilView *view;
    D3D11_TEXTURE2D_DESC desc;
    D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
    ZeroMemory(&desc, sizeof(CD3D11_TEXTURE2D_DESC));
    ZeroMemory(&viewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    desc.Width = (UINT)width;
    desc.Height = (UINT)height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    switch (format)
    {
    case bp3d::driver::EDepthBufferFormat::FLOAT_24_STENCIL_8:
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
    case bp3d::driver::EDepthBufferFormat::FLOAT_32_STENCIL_8:
        desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        break;
    case bp3d::driver::EDepthBufferFormat::FLOAT_32:
        desc.Format = DXGI_FORMAT_D32_FLOAT;
        break;
    }
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    if (FAILED(_device->CreateTexture2D(&desc, Null, &tex)))
        throw bpf::RuntimeException("RenderEngine", "CreateTexture2D failed");
    viewDesc.Format = desc.Format;
    viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipSlice = 0;
    if (FAILED(_device->CreateDepthStencilView(tex, &viewDesc, &view)))
    {
        tex->Release();
        throw bpf::RuntimeException("RenderEngine", "CreateDepthStencilView failed");
    }
    DepthBuffer *buf = static_cast<DepthBuffer *>(bpf::memory::Memory::Malloc(sizeof(DepthBuffer)));
    buf->Texture = tex;
    buf->View = view;
    return (buf);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocRenderTarget(const bp3d::driver::RenderTargetDescriptor &descriptor)
{
    RenderTarget *rt = static_cast<RenderTarget *>(bpf::memory::Memory::Malloc(sizeof(RenderTarget)));
    rt->RTCount = (UINT)descriptor.Components.Size();
    UINT id = 0;
    for (bp3d::driver::Resource res : descriptor.Components)
    {
        Texture2D *tex = reinterpret_cast<Texture2D *>(res);
        D3D11_RENDER_TARGET_VIEW_DESC desc;
        desc.Format = tex->Format;
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;
        ID3D11RenderTargetView *view;
        if (FAILED(_device->CreateRenderTargetView(tex->Texture, &desc, &view)))
        {
            bpf::memory::Memory::Free(rt);
            throw bpf::RuntimeException("RenderEngine", "CreateRenderTargetView failed");
        }
        rt->RTView[id] = view;
        ++id;
    }
    if (descriptor.DepthBuffer != Null)
    {
        DepthBuffer *buf = reinterpret_cast<DepthBuffer *>(descriptor.DepthBuffer);
        rt->DepthView = buf->View;
        rt->DepthView->AddRef();
    }
    return (rt);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocConstantBuffer(const bp3d::driver::EBufferType type, const bp3d::driver::BufferDescriptor &descriptor)
{
    D3D11_BUFFER_DESC desc;
    D3D11_SUBRESOURCE_DATA data;
    ID3D11Buffer *buf;
    desc.ByteWidth = (UINT)descriptor.Size;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        break;
    case bp3d::driver::EBufferType::STATIC:
        desc.CPUAccessFlags = 0;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        break;
    }
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    data.pSysMem = descriptor.Data;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;
    if (FAILED(_device->CreateBuffer(&desc, &data, &buf)))
        throw bpf::RuntimeException("RenderEngine", "CreateBuffer failed");
    return (buf);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocVertexFormat(const bp3d::driver::VertexFormatDescriptor &descriptor)
{
    ID3D11InputLayout *format;
    auto blob = CompileDummyShader(descriptor);
    bpf::collection::Array<bpf::String> temp(descriptor.Components.Size());
    bpf::collection::Array<D3D11_INPUT_ELEMENT_DESC> desc(descriptor.Components.Size());
    UINT offset = 0;
    for (bpf::fsize i = 0; i != descriptor.Components.Size(); ++i)
    {
        temp[i] = descriptor.Components[i].Name.ToUpper();
        desc[i].SemanticName = *temp[i];
        desc[i].SemanticIndex = 0;
        desc[i].AlignedByteOffset = offset;
        desc[i].InputSlot = 0;
        desc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        desc[i].InstanceDataStepRate = 0;
        switch (descriptor.Components[i].Type)
        {
        case bp3d::driver::EVertexComponentType::FLOAT:
            desc[i].Format = DXGI_FORMAT_R32_FLOAT;
            offset += sizeof(FLOAT);
            break;
        case bp3d::driver::EVertexComponentType::INT:
            desc[i].Format = DXGI_FORMAT_R32_SINT;
            offset += sizeof(INT);
            break;
        case bp3d::driver::EVertexComponentType::UINT:
            desc[i].Format = DXGI_FORMAT_R32_UINT;
            offset += sizeof(UINT);
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_2:
            desc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
            offset += sizeof(FLOAT) * 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_3:
            desc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
            offset += sizeof(FLOAT) * 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_4:
            desc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += sizeof(FLOAT) * 4;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_2:
            desc[i].Format = DXGI_FORMAT_R32G32_SINT;
            offset += sizeof(INT) * 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_3:
            desc[i].Format = DXGI_FORMAT_R32G32B32_SINT;
            offset += sizeof(INT) * 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_4:
            desc[i].Format = DXGI_FORMAT_R32G32B32A32_SINT;
            offset += sizeof(INT) * 4;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_2:
            desc[i].Format = DXGI_FORMAT_R32G32_UINT;
            offset += sizeof(UINT) * 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_3:
            desc[i].Format = DXGI_FORMAT_R32G32B32_UINT;
            offset += sizeof(UINT) * 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_4:
            desc[i].Format = DXGI_FORMAT_R32G32B32A32_UINT;
            offset += sizeof(UINT) * 4;
            break;
        }
    }
    if (FAILED(_device->CreateInputLayout(*desc, (UINT)descriptor.Components.Size(), blob->GetBufferPointer(), blob->GetBufferSize(), &format)))
    {
        blob->Release();
        throw bpf::RuntimeException("RenderEngine", "CreateInputLayout failed");
    }
    return (format);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocVertexBuffer(const bp3d::driver::EBufferType type, bp3d::driver::Resource, const bp3d::driver::BufferDescriptor &buffer)
{
    D3D11_BUFFER_DESC desc;
    D3D11_SUBRESOURCE_DATA data;
    ID3D11Buffer *buf;
    ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
    desc.ByteWidth = (UINT)buffer.Size;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        break;
    case bp3d::driver::EBufferType::STATIC:
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;
        break;
    }
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    data.pSysMem = buffer.Data;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;
    if (FAILED(_device->CreateBuffer(&desc, &data, &buf)))
        throw bpf::RuntimeException("RenderEngine", "CreateBuffer failed");
    return (buf);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocIndexBuffer(const bp3d::driver::EBufferType type, const bp3d::driver::BufferDescriptor &descriptor)
{
    D3D11_BUFFER_DESC desc;
    D3D11_SUBRESOURCE_DATA data;
    ID3D11Buffer *buf;
    ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
    desc.ByteWidth = (UINT)descriptor.Size;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        break;
    case bp3d::driver::EBufferType::STATIC:
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;
        break;
    }
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    data.pSysMem = descriptor.Data;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;
    if (FAILED(_device->CreateBuffer(&desc, &data, &buf)))
        throw bpf::RuntimeException("RenderEngine", "CreateBuffer failed");
    return (buf);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocShaderProgram(const bp3d::driver::ShaderProgramDescriptor &descriptor)
{
    ID3D11VertexShader *vertex = Null;
    ID3D11PixelShader *pixel = Null;
    ID3D11GeometryShader *geometry = Null;
    ID3D11HullShader *hull = Null;
    ID3D11DomainShader *domain = Null;
    for (auto &desc : descriptor.Shaders)
    {
        switch (desc.Type)
        {
        case bp3d::driver::EShaderType::GEOMETRY:
            if (FAILED(_device->CreateGeometryShader(desc.Data, desc.Size, Null, &geometry)))
                goto errorCleanup;
            break;
        case bp3d::driver::EShaderType::PIXEL:
            if (FAILED(_device->CreatePixelShader(desc.Data, desc.Size, Null, &pixel)))
                goto errorCleanup;
            break;
        case bp3d::driver::EShaderType::VERTEX:
            if (FAILED(_device->CreateVertexShader(desc.Data, desc.Size, Null, &vertex)))
                goto errorCleanup;
            break;
        case bp3d::driver::EShaderType::HULL:
            if (FAILED(_device->CreateHullShader(desc.Data, desc.Size, Null, &hull)))
                goto errorCleanup;
            break;
        case bp3d::driver::EShaderType::DOMAIN:
            if (FAILED(_device->CreateDomainShader(desc.Data, desc.Size, Null, &domain)))
                goto errorCleanup;
            break;
        }
    }
    goto linkProg;

errorCleanup:
    if (vertex != Null)
        vertex->Release();
    if (pixel != Null)
        pixel->Release();
    if (geometry != Null)
        geometry->Release();
    if (hull != Null)
        hull->Release();
    if (domain != Null)
        domain->Release();
    throw bpf::RuntimeException("RenderEngine", "Could not load shader");

linkProg:
    ShaderProgram *prog = static_cast<ShaderProgram *>(bpf::memory::Memory::Malloc(sizeof(ShaderProgram)));
    for (int i = 0; i != 16; ++i)
    {
        prog->StageFlagsCBuffers[i] = 0;
        prog->StageFlagsTextures[i] = 0;
        prog->StageFlagsSamplers[i] = 0;
    }
    for (auto &bind : descriptor.Bindings)
    {
        switch (bind.Type)
        {
        case bp3d::driver::EBindingType::CONSTANT_BUFFER:
            prog->StageFlagsCBuffers[bind.Register] = bind.StageFlags;
            break;
        case bp3d::driver::EBindingType::SAMPLER:
            prog->StageFlagsSamplers[bind.Register] = bind.StageFlags;
            break;
        case bp3d::driver::EBindingType::TEXTURE:
            prog->StageFlagsTextures[bind.Register] = bind.StageFlags;
            break;
        case bp3d::driver::EBindingType::FIXED_CONSTANT_BUFFER:
            _deviceContext->PSSetConstantBuffers(bind.Register, 1, &_fixedConstBufs[bind.Register]);
            _deviceContext->VSSetConstantBuffers(bind.Register, 1, &_fixedConstBufs[bind.Register]);
            _deviceContext->HSSetConstantBuffers(bind.Register, 1, &_fixedConstBufs[bind.Register]);
            _deviceContext->DSSetConstantBuffers(bind.Register, 1, &_fixedConstBufs[bind.Register]);
            _deviceContext->GSSetConstantBuffers(bind.Register, 1, &_fixedConstBufs[bind.Register]);
            break;
        }
    }
    prog->Geometry = geometry;
    prog->Pixel = pixel;
    prog->Vertex = vertex;
    prog->Domain = domain;
    prog->Hull = hull;
    return (prog);
}

D3D11_BLEND_OP DX11ResourceAllocator::TranslateBlendOp(const bp3d::driver::EBlendOp op)
{
    switch (op)
    {
    case bp3d::driver::EBlendOp::ADD:
        return (D3D11_BLEND_OP_ADD);
    case bp3d::driver::EBlendOp::SUBTRACT:
        return (D3D11_BLEND_OP_SUBTRACT);
    case bp3d::driver::EBlendOp::INVERSE_SUBTRACT:
        return (D3D11_BLEND_OP_REV_SUBTRACT);
    case bp3d::driver::EBlendOp::MIN:
        return (D3D11_BLEND_OP_MIN);
    case bp3d::driver::EBlendOp::MAX:
        return (D3D11_BLEND_OP_MAX);
    }
    return (D3D11_BLEND_OP_ADD);
}

D3D11_BLEND DX11ResourceAllocator::TranslateBlendFactor(const bp3d::driver::EBlendFactor factor)
{
    switch (factor)
    {
    case bp3d::driver::EBlendFactor::ZERO:
        return (D3D11_BLEND_ZERO);
    case bp3d::driver::EBlendFactor::ONE:
        return (D3D11_BLEND_ONE);
    case bp3d::driver::EBlendFactor::DST_ALPHA:
        return (D3D11_BLEND_DEST_ALPHA);
    case bp3d::driver::EBlendFactor::DST_COLOR:
        return (D3D11_BLEND_DEST_COLOR);
    case bp3d::driver::EBlendFactor::ONE_MINUS_DST_ALPHA:
        return (D3D11_BLEND_INV_DEST_ALPHA);
    case bp3d::driver::EBlendFactor::ONE_MINUS_DST_COLOR:
        return (D3D11_BLEND_INV_DEST_COLOR);
    case bp3d::driver::EBlendFactor::ONE_MINUS_SRC_ALPHA:
        return (D3D11_BLEND_INV_SRC_ALPHA);
    case bp3d::driver::EBlendFactor::ONE_MINUS_SRC_COLOR:
        return (D3D11_BLEND_INV_SRC_COLOR);
    case bp3d::driver::EBlendFactor::ONE_MINUS_SRC1_ALPHA:
        return (D3D11_BLEND_INV_SRC1_ALPHA);
    case bp3d::driver::EBlendFactor::ONE_MINUS_SRC1_COLOR:
        return (D3D11_BLEND_INV_SRC1_COLOR);
    case bp3d::driver::EBlendFactor::SRC1_COLOR:
        return (D3D11_BLEND_SRC1_COLOR);
    case bp3d::driver::EBlendFactor::SRC_ALPHA_SATURATE:
        return (D3D11_BLEND_SRC_ALPHA_SAT);
    case bp3d::driver::EBlendFactor::SRC1_ALPHA:
        return (D3D11_BLEND_SRC1_ALPHA);
    case bp3d::driver::EBlendFactor::SRC_COLOR:
        return (D3D11_BLEND_SRC_COLOR);
    case bp3d::driver::EBlendFactor::SRC_ALPHA:
        return (D3D11_BLEND_SRC_ALPHA);
    }
    return (D3D11_BLEND_ZERO);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocBlendState(const bp3d::driver::BlendStateDescriptor &descriptor)
{
    ID3D11BlendState *state;
    D3D11_BLEND_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
    UINT i = 0;
    for (auto &target : descriptor.Components)
    {
        desc.RenderTarget[i].BlendEnable = target.Enable;
        desc.RenderTarget[i].BlendOp = TranslateBlendOp(target.ColorOp);
        desc.RenderTarget[i].BlendOpAlpha = TranslateBlendOp(target.AlphaOp);
        desc.RenderTarget[i].SrcBlend = TranslateBlendFactor(target.SrcColor);
        desc.RenderTarget[i].DestBlend = TranslateBlendFactor(target.DstColor);
        desc.RenderTarget[i].SrcBlendAlpha = TranslateBlendFactor(target.SrcAlpha);
        desc.RenderTarget[i].DestBlendAlpha = TranslateBlendFactor(target.DstAlpha);
        ++i;
    }
    if (FAILED(_device->CreateBlendState(&desc, &state)))
        throw bpf::RuntimeException("RenderEngine", "CreateBlendState failed");
    BlendState *blend = static_cast<BlendState *>(bpf::memory::Memory::Malloc(sizeof(BlendState)));
    blend->BlendState = state;
    blend->Factor = descriptor.Factor;
    return (state);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocPipeline(const bp3d::driver::PipelineDescriptor &descriptor)
{
    ID3D11DepthStencilState *depthState = GetDepthState(descriptor);
    ID3D11RasterizerState *rasterizer = GetRasterizerState(descriptor);
    Pipeline *pipeline = static_cast<Pipeline *>(bpf::memory::Memory::Malloc(sizeof(Pipeline)));
    ShaderProgram *prog = reinterpret_cast<ShaderProgram *>(descriptor.ShaderProgram);
    pipeline->Program = *prog;
    if (pipeline->Program.Domain != Null)
        pipeline->Program.Domain->AddRef();
    if (pipeline->Program.Hull != Null)
        pipeline->Program.Hull->AddRef();
    if (pipeline->Program.Vertex != Null)
        pipeline->Program.Vertex->AddRef();
    if (pipeline->Program.Pixel != Null)
        pipeline->Program.Pixel->AddRef();
    if (pipeline->Program.Geometry != Null)
        pipeline->Program.Geometry->AddRef();
    BlendState *bstate = reinterpret_cast<BlendState *>(descriptor.BlendState);
    if (bstate != Null)
    {
        pipeline->BlendState = bstate->BlendState;
        pipeline->BlendFactor = bstate->Factor;
        pipeline->BlendState->AddRef();
    }
    else
    {
        pipeline->BlendState = Null;
        pipeline->BlendFactor = bpf::math::Vector4f::Zero;
    }
    pipeline->Rasterizer = rasterizer;
    pipeline->DepthState = depthState;
    pipeline->VFormat = reinterpret_cast<ID3D11InputLayout *>(descriptor.VertexFormat);
    pipeline->VFormat->AddRef();
    return (pipeline);
}

bp3d::driver::Resource DX11ResourceAllocator::AllocFixedConstantBuffer(const bp3d::driver::EBufferType type, const int reg, const bp3d::driver::BufferDescriptor &descriptor)
{
    ID3D11Buffer *buffer = reinterpret_cast<ID3D11Buffer *>(AllocConstantBuffer(type, descriptor));
    if ((reg + 1) > _fixedConstBufs.Size())
        _fixedConstBufs.Resize(reg + 1);
    _fixedConstBufs[reg] = buffer;
    return (buffer);
}

void DX11ResourceAllocator::FreeFixedConstantBuffer(bp3d::driver::Resource resource)
{
    ID3D11Buffer *buf = reinterpret_cast<ID3D11Buffer *>(resource);
    for (bpf::fsize i = 0; i != _fixedConstBufs.Size(); ++i)
    {
        if (_fixedConstBufs[i] == buf)
        {
            _fixedConstBufs[i] = Null;
            break;
        }
    }
    buf->Release();
}

void DX11ResourceAllocator::FreePipeline(bp3d::driver::Resource resource)
{
    Pipeline *pipeline = reinterpret_cast<Pipeline *>(resource);
    if (pipeline->BlendState != Null)
        pipeline->BlendState->Release();
    if (pipeline->Program.Domain != Null)
        pipeline->Program.Domain->Release();
    if (pipeline->Program.Hull != Null)
        pipeline->Program.Hull->Release();
    if (pipeline->Program.Geometry != Null)
        pipeline->Program.Geometry->Release();
    if (pipeline->Program.Vertex != Null)
        pipeline->Program.Vertex->Release();
    if (pipeline->Program.Pixel != Null)
        pipeline->Program.Pixel->Release();
    pipeline->VFormat->Release();
    bpf::memory::Memory::Free(pipeline);
}

void DX11ResourceAllocator::FreeShaderProgram(bp3d::driver::Resource resource)
{
    ShaderProgram *prog = reinterpret_cast<ShaderProgram *>(resource);
    if (prog->Domain != Null)
        prog->Domain->Release();
    if (prog->Hull != Null)
        prog->Hull->Release();
    if (prog->Vertex != Null)
        prog->Vertex->Release();
    if (prog->Pixel != Null)
        prog->Pixel->Release();
    if (prog->Geometry != Null)
        prog->Geometry->Release();
    bpf::memory::Memory::Free(prog);
}

void DX11ResourceAllocator::FreeBlendState(bp3d::driver::Resource resource)
{
    BlendState *state = reinterpret_cast<BlendState *>(resource);
    state->BlendState->Release();
    bpf::memory::Memory::Free(state);
}

void DX11ResourceAllocator::FreeVertexFormat(bp3d::driver::Resource resource)
{
    ID3D11InputLayout *format = reinterpret_cast<ID3D11InputLayout *>(resource);
    format->Release();
}

void DX11ResourceAllocator::FreeDepthBuffer(bp3d::driver::Resource resource)
{
    DepthBuffer *buf = reinterpret_cast<DepthBuffer *>(resource);
    buf->Texture->Release();
    buf->View->Release();
    bpf::memory::Memory::Free(buf);
}

void DX11ResourceAllocator::FreeRenderTargetComponent(bp3d::driver::Resource resource)
{
    FreeTexture2D(resource);
}

void DX11ResourceAllocator::FreeTexture2D(bp3d::driver::Resource resource)
{
    Texture2D *tex = reinterpret_cast<Texture2D *>(resource);
    tex->Texture->Release();
    if (tex->View != Null)
        tex->View->Release();
    bpf::memory::Memory::Free(tex);
}

void DX11ResourceAllocator::FreeTexture2DArray(bp3d::driver::Resource resource)
{
    Texture2D *tex = reinterpret_cast<Texture2D *>(resource);
    tex->Texture->Release();
    tex->View->Release();
    bpf::memory::Memory::Free(tex);
}

void DX11ResourceAllocator::FreeTextureCube(bp3d::driver::Resource resource)
{
    Texture2D *tex = reinterpret_cast<Texture2D *>(resource);
    tex->Texture->Release();
    tex->View->Release();
    bpf::memory::Memory::Free(tex);
}

void DX11ResourceAllocator::FreeSampler(bp3d::driver::Resource resource)
{
    ID3D11SamplerState *state = reinterpret_cast<ID3D11SamplerState *>(resource);
    state->Release();
}

void DX11ResourceAllocator::FreeRenderTarget(bp3d::driver::Resource resource)
{
    RenderTarget *rt = reinterpret_cast<RenderTarget *>(resource);
    for (bpf::fsize i = 0; i != rt->RTCount; ++i)
        rt->RTView[i]->Release();
    if (rt->DepthView != Null)
        rt->DepthView->Release();
    bpf::memory::Memory::Free(rt);
}

void DX11ResourceAllocator::FreeConstantBuffer(bp3d::driver::Resource resource)
{
    ID3D11Buffer *buf = reinterpret_cast<ID3D11Buffer *>(resource);
    buf->Release();
}

void DX11ResourceAllocator::FreeVertexBuffer(bp3d::driver::Resource resource)
{
    ID3D11Buffer *buf = reinterpret_cast<ID3D11Buffer *>(resource);
    buf->Release();
}

void DX11ResourceAllocator::FreeIndexBuffer(bp3d::driver::Resource resource)
{
    ID3D11Buffer *buf = reinterpret_cast<ID3D11Buffer *>(resource);
    buf->Release();
}
