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
    : _ra(dev, devContext)
    , _device(dev)
    , _deviceContext(devContext)
    , _curRT(Null)
    , _backDepthBuffer(Null)
    , _backBuffer(Null)
    , _backBufferView(Null)
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = true;
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
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDesc, &_enableDepth)))
        throw bpf::RuntimeException("RenderEngine", "CreateDepthStencilState failed");
    depthStencilDesc.DepthEnable = false;
    if (FAILED(_device->CreateDepthStencilState(&depthStencilDesc, &_disableDepth)))
        throw bpf::RuntimeException("RenderEngine", "CreateDepthStencilState failed");
    for (UINT i = 0; i != 12; ++i)
        _states[i] = Null;
    _curCullMode = D3D11_CULL_NONE;
    _curFillMode = D3D11_FILL_SOLID;
    _curScissor = FALSE;
    _baseDesc.FillMode = _curFillMode;
    _baseDesc.CullMode = _curCullMode;
    _baseDesc.FrontCounterClockwise = TRUE;
    _baseDesc.DepthBias = 0;
    _baseDesc.DepthBiasClamp = 0;
    _baseDesc.DepthClipEnable = TRUE;
    _baseDesc.ScissorEnable = _curScissor;
    _baseDesc.MultisampleEnable = rprops.Multisampling ? TRUE : FALSE;
    _baseDesc.AntialiasedLineEnable = rprops.Antialiasing ? TRUE : FALSE;
    UINT state = DX11_RASTERIZER_INDEX(_curCullMode, _curScissor, _curFillMode);
    if (FAILED(_device->CreateRasterizerState(&_baseDesc, &_states[state])))
        throw bpf::RuntimeException("RenderEngine", "CreateRasterizerState failed");
    _deviceContext->RSSetState(_states[state]);
    _deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

DX11RenderContext::~DX11RenderContext()
{
    for (UINT i = 0; i != 12; ++i)
    {
        if (_states[i] != Null)
            _states[i]->Release();
    }
    _enableDepth->Release();
    _disableDepth->Release();
    _backBufferView->Release();
    _backBuffer->Release();
}

void DX11RenderContext::InitCurState()
{
    UINT state = DX11_RASTERIZER_INDEX(_curCullMode, _curScissor, _curFillMode);
    D3D11_RASTERIZER_DESC desc = _baseDesc;
    desc.CullMode = _curCullMode;
    desc.FillMode = _curFillMode;
    desc.ScissorEnable = _curScissor;
    _device->CreateRasterizerState(&desc, &_states[state]);
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

void DX11RenderContext::LockConstantBuffer(bp3d::driver::Resource resource, const bpf::fint reg, const int stageFlags) noexcept
{
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

void DX11RenderContext::LockTexture(bp3d::driver::Resource resource, const bpf::fint reg, const int stageFlags) noexcept
{
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

void DX11RenderContext::LockSampler(bp3d::driver::Resource resource, const bpf::fint reg, const int stageFlags) noexcept
{
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

void DX11RenderContext::LockVertexFormat(bp3d::driver::Resource resource) noexcept
{
    auto var = reinterpret_cast<ID3D11InputLayout *>(resource);
    _deviceContext->IASetInputLayout(var);
}

void DX11RenderContext::LockShaderProgram(bp3d::driver::Resource resource, const int stageFlags) noexcept
{
    auto var = reinterpret_cast<ShaderProgram *>(resource);
    if (stageFlags & bp3d::driver::LOCK_GEOMETRY_STAGE)
        _deviceContext->GSSetShader(var->Geometry, Null, 0);
    if (stageFlags & bp3d::driver::LOCK_PIXEL_STAGE)
        _deviceContext->PSSetShader(var->Pixel, Null, 0);
    if (stageFlags & bp3d::driver::LOCK_VERTEX_STAGE)
        _deviceContext->VSSetShader(var->Vertex, Null, 0);
    if (stageFlags & bp3d::driver::LOCK_HULL_STAGE)
        _deviceContext->HSSetShader(var->Hull, Null, 0);
    if (stageFlags & bp3d::driver::LOCK_DOMAIN_STAGE)
        _deviceContext->DSSetShader(var->Domain, Null, 0);
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

void DX11RenderContext::SetRenderMode(const bp3d::driver::ERenderMode mode) noexcept
{
    switch (mode)
    {
    case bp3d::driver::ERenderMode::TRIANGLES:
        _curFillMode = D3D11_FILL_SOLID;
        break;
    case bp3d::driver::ERenderMode::WIREFRAME:
        _curFillMode = D3D11_FILL_WIREFRAME;
        break;
    }
    UINT state = DX11_RASTERIZER_INDEX(_curCullMode, _curScissor, _curFillMode);
    if (_states[state] == Null)
        InitCurState();
    _deviceContext->RSSetState(_states[state]);
}

void DX11RenderContext::EnableDepthTest(const bool flag) noexcept
{
    if (flag)
        _deviceContext->OMSetDepthStencilState(_enableDepth, 0);
    else
        _deviceContext->OMSetDepthStencilState(_disableDepth, 0);
}

void DX11RenderContext::SetCullingMode(const bp3d::driver::ECullingMode mode) noexcept
{
    switch (mode)
    {
    case bp3d::driver::ECullingMode::BACK_FACE:
        _curCullMode = D3D11_CULL_BACK;
        break;
    case bp3d::driver::ECullingMode::FRONT_FACE:
        _curCullMode = D3D11_CULL_FRONT;
        break;
    case bp3d::driver::ECullingMode::DISABLED:
        _curCullMode = D3D11_CULL_NONE;
        break;
    }
    UINT state = DX11_RASTERIZER_INDEX(_curCullMode, _curScissor, _curFillMode);
    if (_states[state] == Null)
        InitCurState();
    _deviceContext->RSSetState(_states[state]);
}

void DX11RenderContext::LockBlendState(bp3d::driver::Resource resource, const bpf::math::Vector4f &factor) noexcept
{
    auto state = reinterpret_cast<ID3D11BlendState *>(resource);
    _deviceContext->OMSetBlendState(state, *factor, 0xffffffff);
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

void DX11RenderContext::EnableScissor(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept
{
    _curScissor = TRUE;
    UINT state = DX11_RASTERIZER_INDEX(_curCullMode, _curScissor, _curFillMode);
    if (_states[state] == Null)
        InitCurState();
    _deviceContext->RSSetState(_states[state]);
    D3D11_RECT rect;
    rect.left = x;
    rect.top = y;
    rect.bottom = (LONG)(y + h);
    rect.right = (LONG)(x + w);
    _deviceContext->RSSetScissorRects(1, &rect);
}

void DX11RenderContext::DisableScissor() noexcept
{
    _curScissor = FALSE;
    UINT state = DX11_RASTERIZER_INDEX(_curCullMode, _curScissor, _curFillMode);
    if (_states[state] == Null)
        InitCurState();
    _deviceContext->RSSetState(_states[state]);
}
