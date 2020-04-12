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

#include "DX11RenderContext.hpp"
#include "DX11Resources.hpp"
#include <Framework/RuntimeException.hpp>

using namespace dx11;

DX11RenderContext::DX11RenderContext(ID3D11Device *dev, ID3D11DeviceContext *devContext, const bp3d::driver::RenderProperties &rprops)
    : _deviceContext(devContext)
    , _curPipeline(Null)
    , _curRT(Null)
    , _backBufferView(Null)
    , _backDepthBuffer(Null)
    , _backBuffer(Null)
    , _ra(dev, devContext, rprops)
    , _device(dev)
{
    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

DX11RenderContext::~DX11RenderContext()
{
    _backBufferView->Release();
    _backBuffer->Release();
}

void DX11RenderContext::UpdateBackBuffer()
{
    if (FAILED(_device->CreateRenderTargetView(_backBuffer, NULL, &_backBufferView)))
        throw bpf::RuntimeException("RenderEngine", "CreateRenderTargetView failed");
}

void DX11RenderContext::SetRenderTarget(bp3d::driver::Resource resource) noexcept
{
    RenderTarget *oldRT = reinterpret_cast<RenderTarget *>(_curRT);
    RenderTarget *rt = reinterpret_cast<RenderTarget *>(resource);
    if (resource == Null)
    {
        if (_curRT == Null)
            _deviceContext->OMSetRenderTargets(1, &_backBufferView, reinterpret_cast<DepthBuffer *>(_backDepthBuffer)->View);
        else
        {
            if (oldRT->RTCount > 1)
            {
                ID3D11RenderTargetView *views[8] = { Null };
                views[0] = _backBufferView;
                _deviceContext->OMSetRenderTargets(oldRT->RTCount, views, reinterpret_cast<DepthBuffer *>(_backDepthBuffer)->View);
            }
            else
                _deviceContext->OMSetRenderTargets(1, &_backBufferView, reinterpret_cast<DepthBuffer *>(_backDepthBuffer)->View);
        }
    }
    else
    {
        if (_curRT == Null)
            _deviceContext->OMSetRenderTargets(rt->RTCount, rt->RTView, rt->DepthView);
        else
        {
            if (oldRT->RTCount > rt->RTCount)
            {
                ID3D11RenderTargetView *views[8] = { Null };
                for (bpf::fsize i = 0; i != rt->RTCount; ++i)
                    views[i] = rt->RTView[i];
                _deviceContext->OMSetRenderTargets(oldRT->RTCount, views, rt->DepthView);
            }
            else
                _deviceContext->OMSetRenderTargets(rt->RTCount, rt->RTView, rt->DepthView);
        }
    }
    _curRT = rt;
}

bp3d::driver::Resource DX11RenderContext::GetRenderTarget() noexcept
{
    return (_curRT);
}

void DX11RenderContext::LockConstantBuffer(bp3d::driver::Resource resource, const bpf::fint reg) noexcept
{
    int stageFlags = _curPipeline == Null ? 0 : _curPipeline->Program.StageFlagsCBuffers[reg];
    auto var = reinterpret_cast<ID3D11Buffer *>(resource);
    if (stageFlags & bp3d::driver::LOCK_GEOMETRY_STAGE)
        _deviceContext->GSSetConstantBuffers(reg, 1, &var);
    if (stageFlags & bp3d::driver::LOCK_PIXEL_STAGE)
        _deviceContext->PSSetConstantBuffers(reg, 1, &var);
    if (stageFlags & bp3d::driver::LOCK_VERTEX_STAGE)
        _deviceContext->VSSetConstantBuffers(reg, 1, &var);
    if (stageFlags & bp3d::driver::LOCK_HULL_STAGE)
        _deviceContext->HSSetConstantBuffers(reg, 1, &var);
    if (stageFlags & bp3d::driver::LOCK_DOMAIN_STAGE)
        _deviceContext->DSSetConstantBuffers(reg, 1, &var);
}

void DX11RenderContext::UpdateConstantBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept
{
    auto var = reinterpret_cast<ID3D11Buffer *>(resource);
    D3D11_MAPPED_SUBRESOURCE res;
    _deviceContext->Map(var, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    std::memcpy(res.pData, data, size);
    _deviceContext->Unmap(var, 0);
}

void DX11RenderContext::LockTexture(bp3d::driver::Resource resource, const bpf::fint reg) noexcept
{
    int stageFlags = _curPipeline == Null ? 0 : _curPipeline->Program.StageFlagsTextures[reg];
    auto var = reinterpret_cast<Texture2D *>(resource);
    if (stageFlags & bp3d::driver::LOCK_GEOMETRY_STAGE)
        _deviceContext->GSSetShaderResources(reg, 1, &var->View);
    if (stageFlags & bp3d::driver::LOCK_PIXEL_STAGE)
        _deviceContext->PSSetShaderResources(reg, 1, &var->View);
    if (stageFlags & bp3d::driver::LOCK_VERTEX_STAGE)
        _deviceContext->VSSetShaderResources(reg, 1, &var->View);
    if (stageFlags & bp3d::driver::LOCK_HULL_STAGE)
        _deviceContext->HSSetShaderResources(reg, 1, &var->View);
    if (stageFlags & bp3d::driver::LOCK_DOMAIN_STAGE)
        _deviceContext->DSSetShaderResources(reg, 1, &var->View);
}

void DX11RenderContext::UpdateTexture(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept
{
    auto var = reinterpret_cast<Texture2D *>(resource);
    D3D11_MAPPED_SUBRESOURCE res;
    _deviceContext->Map(var->Texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    std::memcpy(res.pData, data, size);
    _deviceContext->Unmap(var->Texture, 0);
}

void DX11RenderContext::LockSampler(bp3d::driver::Resource resource, const bpf::fint reg) noexcept
{
    int stageFlags = _curPipeline == Null ? 0 : _curPipeline->Program.StageFlagsSamplers[reg];
    auto var = reinterpret_cast<ID3D11SamplerState *>(resource);
    if (stageFlags & bp3d::driver::LOCK_GEOMETRY_STAGE)
        _deviceContext->GSSetSamplers(reg, 1, &var);
    if (stageFlags & bp3d::driver::LOCK_PIXEL_STAGE)
        _deviceContext->PSSetSamplers(reg, 1, &var);
    if (stageFlags & bp3d::driver::LOCK_VERTEX_STAGE)
        _deviceContext->VSSetSamplers(reg, 1, &var);
    if (stageFlags & bp3d::driver::LOCK_HULL_STAGE)
        _deviceContext->HSSetSamplers(reg, 1, &var);
    if (stageFlags & bp3d::driver::LOCK_DOMAIN_STAGE)
        _deviceContext->DSSetSamplers(reg, 1, &var);
}

void DX11RenderContext::LockIndexBuffer(bp3d::driver::Resource resource) noexcept
{
    auto var = reinterpret_cast<ID3D11Buffer *>(resource);
    _deviceContext->IASetIndexBuffer(var, DXGI_FORMAT_R32_UINT, 0);
}

void DX11RenderContext::UpdateIndexBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept
{
    auto var = reinterpret_cast<ID3D11Buffer *>(resource);
    D3D11_MAPPED_SUBRESOURCE res;
    _deviceContext->Map(var, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    std::memcpy(res.pData, data, size);
    _deviceContext->Unmap(var, 0);
}

void DX11RenderContext::LockVertexBuffer(bp3d::driver::Resource resource, const bpf::uint32 vertexSize) noexcept
{
    auto var = reinterpret_cast<ID3D11Buffer *>(resource);
    UINT offset = 0;
    _deviceContext->IASetVertexBuffers(0, 1, &var, &vertexSize, &offset);
}

void DX11RenderContext::UpdateVertexBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept
{
    auto var = reinterpret_cast<ID3D11Buffer *>(resource);
    D3D11_MAPPED_SUBRESOURCE res;
    _deviceContext->Map(var, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    std::memcpy(res.pData, data, size);
    _deviceContext->Unmap(var, 0);
}

void DX11RenderContext::LockPipeline(bp3d::driver::Resource resource) noexcept
{
    if (resource == _curPipeline)
        return;
    if (resource == Null)
    {
        _curPipeline = Null;
        return;
    }
    Pipeline *pipeline = reinterpret_cast<Pipeline *>(resource);
    if (_curPipeline == Null)
    {
        _deviceContext->RSSetState(pipeline->Rasterizer);
        _deviceContext->OMSetDepthStencilState(pipeline->DepthState, 0);
        _deviceContext->IASetInputLayout(pipeline->VFormat);
        if (pipeline->BlendState != Null)
            _deviceContext->OMSetBlendState(pipeline->BlendState, *pipeline->BlendFactor, 0);
        _deviceContext->PSSetShader(pipeline->Program.Pixel, Null, 0);
        _deviceContext->VSSetShader(pipeline->Program.Vertex, Null, 0);
        _deviceContext->GSSetShader(pipeline->Program.Geometry, Null, 0);
        _deviceContext->HSSetShader(pipeline->Program.Hull, Null, 0);
        _deviceContext->DSSetShader(pipeline->Program.Domain, Null, 0);
    }
    else
    {
        if (_curPipeline->Rasterizer != pipeline->Rasterizer)
            _deviceContext->RSSetState(pipeline->Rasterizer);
        if (_curPipeline->DepthState != pipeline->DepthState)
            _deviceContext->OMSetDepthStencilState(pipeline->DepthState, 0);
        if (_curPipeline->VFormat != pipeline->VFormat)
            _deviceContext->IASetInputLayout(pipeline->VFormat);
        if (pipeline->BlendState != Null && _curPipeline->BlendState != pipeline->BlendState)
            _deviceContext->OMSetBlendState(pipeline->BlendState, *pipeline->BlendFactor, 0);
        if (_curPipeline->Program.Pixel != pipeline->Program.Pixel)
            _deviceContext->PSSetShader(pipeline->Program.Pixel, Null, 0);
        if (_curPipeline->Program.Vertex != pipeline->Program.Vertex)
            _deviceContext->VSSetShader(pipeline->Program.Vertex, Null, 0);
        if (_curPipeline->Program.Geometry != pipeline->Program.Geometry)
            _deviceContext->GSSetShader(pipeline->Program.Geometry, Null, 0);
        if (_curPipeline->Program.Hull != pipeline->Program.Hull)
            _deviceContext->HSSetShader(pipeline->Program.Hull, Null, 0);
        if (_curPipeline->Program.Domain != pipeline->Program.Domain)
            _deviceContext->DSSetShader(pipeline->Program.Domain, Null, 0);
    }
    _curPipeline = pipeline;
}

void DX11RenderContext::Draw(const bpf::uint32 index, const bpf::uint32 count) noexcept
{
    _deviceContext->Draw(count, index);
}

void DX11RenderContext::DrawInstanced(const bpf::uint32 index, const bpf::uint32 count, const bpf::uint32 instanceCount) noexcept
{
    _deviceContext->DrawInstanced(count, instanceCount, index, 0);
}

void DX11RenderContext::DrawIndexed(const bpf::uint32 index, const bpf::uint32 count) noexcept
{
    _deviceContext->DrawIndexed(count, index, 0);
}

void DX11RenderContext::DrawPatches(const bpf::uint32 index, const bpf::uint32 count, const bpf::uint8 controlPoints) noexcept
{
    _deviceContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)(D3D11_PRIMITIVE_1_CONTROL_POINT_PATCH + (controlPoints - 1)));
    _deviceContext->Draw(count, index);
    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DX11RenderContext::Clear(const bool colorBuffer, const bool depthBuffer) noexcept
{
    if (_curRT != Null)
    {
        auto var = reinterpret_cast<RenderTarget *>(_curRT);
        if (colorBuffer)
        {
            FLOAT v[4] = { 0, 0, 0, 0 };
            for (UINT i = 0; i != var->RTCount; ++i)
                _deviceContext->ClearRenderTargetView(var->RTView[i], v);
        }
        if (depthBuffer)
            _deviceContext->ClearDepthStencilView(var->DepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
    else
    {
        if (colorBuffer)
        {
            FLOAT v[4] = { 0, 0, 0, 0 };
            _deviceContext->ClearRenderTargetView(_backBufferView, v);
        }
        if (depthBuffer)
            _deviceContext->ClearDepthStencilView(reinterpret_cast<DepthBuffer *>(_backDepthBuffer)->View, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
}

bool DX11RenderContext::ReadPixels(void *, const bpf::fint, const bpf::fint, const bpf::fsize, const bpf::fsize) noexcept
{
    //Not implemented yet: amazingly complex to do under DX11
    return (false);
}

void DX11RenderContext::SetViewport(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept
{
    D3D11_VIEWPORT vp;
    vp.TopLeftX = (FLOAT)x;
    vp.TopLeftY = (FLOAT)y;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.Width = (FLOAT)w;
    vp.Height = (FLOAT)h;
    _deviceContext->RSSetViewports(1, &vp);
}

void DX11RenderContext::SetScissor(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept
{
    D3D11_RECT rect;
    rect.left = x;
    rect.top = y;
    rect.bottom = (LONG)(y + h);
    rect.right = (LONG)(x + w);
    _deviceContext->RSSetScissorRects(1, &rect);
}
