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
#include <Engine/Driver/IRenderContext.hpp>
#include <Engine/Driver/IRenderEngine.hpp>
#include "DX11ResourceAllocator.hpp"

#define DX11_RASTERIZER_INDEX(cull, scissor, fill) (((UINT)cull - 1) + ((UINT)scissor * 3) + ((UINT)fill - 2) * 6)

namespace dx11
{
    class DX11RenderContext final : public bp3d::driver::IRenderContext
    {
    private:
        DX11ResourceAllocator _ra;
        ID3D11Device *_device;
        ID3D11DeviceContext *_deviceContext;
        bp3d::driver::Resource _curRT;
        bp3d::driver::Resource _backDepthBuffer;
        ID3D11Texture2D *_backBuffer;
        ID3D11RenderTargetView *_backBufferView;
        ID3D11DepthStencilState *_enableDepth;
        ID3D11DepthStencilState *_disableDepth;
        ID3D11RasterizerState *_states[12]; //Formula: cullmodeState + (scissorState * 3) + (fillState * 6)
        D3D11_CULL_MODE _curCullMode;
        D3D11_FILL_MODE _curFillMode;
        BOOL _curScissor;
        D3D11_RASTERIZER_DESC _baseDesc;

        void UpdateBackBuffer();
        void InitCurState();
    public:
        DX11RenderContext(ID3D11Device *dev, ID3D11DeviceContext *devContext, const bp3d::driver::RenderProperties &rprops);
        ~DX11RenderContext();

        inline bp3d::driver::IResourceAllocator &GetResourceAllocator() noexcept
        {
            return (_ra);
        }

        inline void SetBackBuffer(bp3d::driver::Resource depth, ID3D11Texture2D *buf)
        {
            bool flag = false;
            if (_backBuffer == Null)
                flag = true;
            _backDepthBuffer = depth;
            _backBuffer = buf;
            if (flag)
                UpdateBackBuffer();
        }

        void LockConstantBuffer(bp3d::driver::Resource resource, const bpf::fint reg, const int stageFlags) noexcept;
        void UpdateConstantBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept;
        void LockTexture(bp3d::driver::Resource resource, const bpf::fint reg, const int stageFlags) noexcept;
        void UpdateTexture(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept;
        void LockSampler(bp3d::driver::Resource resource, const bpf::fint reg, const int stageFlags) noexcept;
        void SetRenderTarget(bp3d::driver::Resource resource) noexcept;
        bp3d::driver::Resource GetRenderTarget() noexcept;
        void LockIndexBuffer(bp3d::driver::Resource resource) noexcept;
        void UpdateIndexBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept;
        void LockVertexBuffer(bp3d::driver::Resource resource, const bpf::uint32 vertexSize) noexcept;
        void UpdateVertexBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept;
        void LockVertexFormat(bp3d::driver::Resource resource) noexcept;
        void LockShaderProgram(bp3d::driver::Resource resource, const int stageFlags) noexcept;
        void LockBlendState(bp3d::driver::Resource resource, const bpf::math::Vector4f &factor) noexcept;
        void DrawPatches(const bpf::uint32 index, const bpf::uint32 count, const bpf::uint8 controlPoints) noexcept;
        void Draw(const bpf::uint32 index, const bpf::uint32 count) noexcept;
        void DrawInstanced(const bpf::uint32 index, const bpf::uint32 count, const bpf::uint32 instanceCount) noexcept;
        void DrawIndexed(const bpf::uint32 index, const bpf::uint32 count) noexcept;
        void Clear(const bool colorBuffer = false, const bool depthBuffer = true) noexcept;
        bool ReadPixels(void *output, const bpf::fint x, const bpf::fint y, const bpf::fsize w, const bpf::fsize h) noexcept;
        void SetRenderMode(const bp3d::driver::ERenderMode mode) noexcept; //D3D11_RASTERIZER_DESC
        void EnableDepthTest(const bool flag) noexcept; //D3D11_DEPTH_STENCIL_DESC
        void SetCullingMode(const bp3d::driver::ECullingMode mode) noexcept; //D3D11_RASTERIZER_DESC
        void SetViewport(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept;
        void EnableScissor(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept;
        void DisableScissor() noexcept;
    };
}
