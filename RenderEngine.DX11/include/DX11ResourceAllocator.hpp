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
#include <d3d11.h>
#include <d3dcompiler.h>
#include <Framework/Collection/HashMap.hpp>
#include <Engine/Driver/IResourceAllocator.hpp>
#include <Engine/Driver/IRenderEngine.hpp>

#define DX11_RASTERIZER_INDEX(cull, scissor, fill) (((UINT)cull - 1) + ((UINT)scissor * 3) + ((UINT)fill - 2) * 6)
#define DX11_DEPTHSTATE_INDEX(depthEnable, depthWriteEnable) ((UINT)depthEnable + ((UINT) depthWriteEnable * 2))

namespace dx11
{
    class DX11ResourceAllocator final : public bp3d::driver::IResourceAllocator
    {
    private:
        ID3D11Device *_device;
        ID3D11DeviceContext *_deviceContext;
        ID3D11DepthStencilState *_depthStates[4]; //Formula depthEnable + (depthWriteEnable * 2)
        ID3D11RasterizerState *_states[12]; //Formula: cullmodeState + (scissorState * 3) + (fillState * 6)
        D3D11_RASTERIZER_DESC _baseDesc;
        bpf::collection::Array<ID3D11Buffer *> _fixedConstBufs;

        void SetupTextureFormat(const bp3d::driver::TextureDescriptor &descriptor, D3D11_TEXTURE2D_DESC &desc, UINT &sysmempitch, UINT &sysmemslicepitch, D3D11_SHADER_RESOURCE_VIEW_DESC &shaderDesc);
        ID3DBlob *CompileDummyShader(const bp3d::driver::VertexFormatDescriptor &descriptor);
        D3D11_BLEND_OP TranslateBlendOp(const bp3d::driver::EBlendOp op);
        D3D11_BLEND TranslateBlendFactor(const bp3d::driver::EBlendFactor factor);
        ID3D11RasterizerState *GetRasterizerState(const bp3d::driver::PipelineDescriptor &descriptor);
        ID3D11DepthStencilState *GetDepthState(const bp3d::driver::PipelineDescriptor &descriptor);

    public:
        DX11ResourceAllocator(ID3D11Device *dev, ID3D11DeviceContext *devContext, const bp3d::driver::RenderProperties &rprops);
        ~DX11ResourceAllocator();

        bp3d::driver::Resource AllocDepthBuffer(const bpf::fsize width, const bpf::fsize height, const bp3d::driver::EDepthBufferFormat format);
        bp3d::driver::Resource AllocTexture2D(const bp3d::driver::EBufferType type, const bp3d::driver::TextureDescriptor &descriptor);
        bp3d::driver::Resource AllocTexture2DArray(const bp3d::driver::EBufferType type, const bp3d::driver::TextureDescriptor &descriptor, const bpf::fsize layers);
        bp3d::driver::Resource AllocTextureCube(const bp3d::driver::EBufferType type, const bp3d::driver::TextureDescriptor &descriptor);
        bp3d::driver::Resource AllocSampler(const bp3d::driver::SamplerDescriptor &descriptor);
        bp3d::driver::Resource AllocRenderTargetComponent(const bp3d::driver::RenderTargetComponentDescriptor &descriptor);
        bp3d::driver::Resource AllocRenderTarget(const bp3d::driver::RenderTargetDescriptor &descriptor);
        bp3d::driver::Resource AllocConstantBuffer(const bp3d::driver::EBufferType type, const bp3d::driver::BufferDescriptor &descriptor);
        bp3d::driver::Resource AllocFixedConstantBuffer(const bp3d::driver::EBufferType type, const int reg, const bp3d::driver::BufferDescriptor &descriptor);
        bp3d::driver::Resource AllocVertexFormat(const bp3d::driver::VertexFormatDescriptor &descriptor);
        bp3d::driver::Resource AllocVertexBuffer(const bp3d::driver::EBufferType type, bp3d::driver::Resource vformat, const bp3d::driver::BufferDescriptor &buffer);
        bp3d::driver::Resource AllocIndexBuffer(const bp3d::driver::EBufferType type, const bp3d::driver::BufferDescriptor &descriptor);
        bp3d::driver::Resource AllocShaderProgram(const bp3d::driver::ShaderProgramDescriptor &descriptor);
        bp3d::driver::Resource AllocBlendState(const bp3d::driver::BlendStateDescriptor &descriptor);
        bp3d::driver::Resource AllocPipeline(const bp3d::driver::PipelineDescriptor &descriptor);
        void FreeFixedConstantBuffer(bp3d::driver::Resource resource);
        void FreeVertexFormat(bp3d::driver::Resource resource);
        void FreePipeline(bp3d::driver::Resource resource);
        void FreeDepthBuffer(bp3d::driver::Resource resource);
        void FreeTexture2D(bp3d::driver::Resource resource);
        void FreeTexture2DArray(bp3d::driver::Resource resource);
        void FreeTextureCube(bp3d::driver::Resource resource);
        void FreeSampler(bp3d::driver::Resource resource);
        void FreeRenderTargetComponent(bp3d::driver::Resource resource);
        void FreeRenderTarget(bp3d::driver::Resource resource);
        void FreeConstantBuffer(bp3d::driver::Resource resource);
        void FreeVertexBuffer(bp3d::driver::Resource resource);
        void FreeIndexBuffer(bp3d::driver::Resource resource);
        void FreeShaderProgram(bp3d::driver::Resource resource);
        void FreeBlendState(bp3d::driver::Resource resource);
    };
}
