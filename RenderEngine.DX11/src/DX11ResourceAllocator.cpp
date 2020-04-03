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
        }
        str += comp.Name + ":" + comp.Name.ToUpper() + ";";
    }
    bpf::String inputStruct = bpf::String("struct ") + descriptor.Name + '{' +  str + "};";
    bpf::String outputStruct = bpf::String("struct DummyOutput{float4 Dummy: SV_POSITION;") + str + "};";
    bpf::String mainFunc = bpf::String("DummyOutput main(") + descriptor.Name + " input){DummyOutput output;output.Dummy=float4(0,0,0,0);";
    for (auto &comp : descriptor.Components)
        mainFunc += bpf::String("output.") + comp.Name + '=' + "input." + comp.Name + ';';
    mainFunc += "return (output);}";
    bpf::String dummyShaderCode = inputStruct + outputStruct + mainFunc;
    D3D_SHADER_MACRO dummy[] = { Null, Null };
    ID3DBlob *blob;
    if (FAILED(D3DCompile(*dummyShaderCode, dummyShaderCode.Size(), Null, dummy, Null, "main", Null, D3DCOMPILE_SKIP_VALIDATION, 0, &blob, Null)))
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
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
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

bp3d::driver::Resource DX11ResourceAllocator::AllocDepthBuffer(const bpf::fsize width, const bpf::fsize height)
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
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
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
    for (bpf::fsize i = 0; i != descriptor.Components.Size(); ++i)
    {
        temp[i] = descriptor.Components[i].Name.ToUpper();
        desc[i].SemanticName = *temp[i];
        desc[i].SemanticIndex = 0;
        switch (descriptor.Components[i].Type)
        {
        case bp3d::driver::EVertexComponentType::FLOAT:
            desc[i].Format = DXGI_FORMAT_R32_FLOAT;
            break;
        case bp3d::driver::EVertexComponentType::INT:
            desc[i].Format = DXGI_FORMAT_R32_SINT;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_2:
            desc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_3:
            desc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_4:
            desc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_2:
            desc[i].Format = DXGI_FORMAT_R32G32_SINT;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_3:
            desc[i].Format = DXGI_FORMAT_R32G32B32_SINT;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_4:
            desc[i].Format = DXGI_FORMAT_R32G32B32A32_SINT;
            break;
        }
        desc[i].InputSlot = (UINT)i;
        desc[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        desc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        desc[i].InstanceDataStepRate = 0;
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
    for (auto &desc : descriptor.Shaders)
    {
        switch (desc.Type)
        {
        case bp3d::driver::EShaderType::GEOMETRY:
            if (FAILED(_device->CreateGeometryShader(desc.Data, desc.Size, Null, &geometry)))
                goto errorCleanup;
            goto linkProg;
        case bp3d::driver::EShaderType::PIXEL:
            if (FAILED(_device->CreatePixelShader(desc.Data, desc.Size, Null, &pixel)))
                goto errorCleanup;
            goto linkProg;
        case bp3d::driver::EShaderType::VERTEX:
            if (FAILED(_device->CreateVertexShader(desc.Data, desc.Size, Null, &vertex)))
                goto errorCleanup;
            goto linkProg;
        }
    }

errorCleanup:
    if (vertex != Null)
        vertex->Release();
    if (pixel != Null)
        pixel->Release();
    if (geometry != Null)
        geometry->Release();
    throw bpf::RuntimeException("RenderEngine", "Could not load shader");

linkProg:
    ShaderProgram *prog = static_cast<ShaderProgram *>(bpf::memory::Memory::Malloc(sizeof(ShaderProgram)));
    prog->Geometry = geometry;
    prog->Pixel = pixel;
    prog->Vertex = vertex;
    return (prog);
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

void DX11ResourceAllocator::FreeShaderProgram(bp3d::driver::Resource resource)
{
    ShaderProgram *prog = reinterpret_cast<ShaderProgram *>(resource);
    if (prog->Geometry != Null)
        prog->Geometry->Release();
    if (prog->Pixel != Null)
        prog->Pixel->Release();
    if (prog->Vertex != Null)
        prog->Vertex->Release();
    bpf::memory::Memory::Free(prog);
}
