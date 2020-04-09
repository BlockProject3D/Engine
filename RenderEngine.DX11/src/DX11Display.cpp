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

#include "DX11Display.hpp"
#ifdef BUILD_DEBUG
#include <dxgi1_3.h>
#include <dxgidebug.h>
#endif

using namespace dx11;

DX11StandardDisplay::DX11StandardDisplay(IDXGISwapChain *swap, ID3D11Device *dev, ID3D11DeviceContext *devContext, const bp3d::driver::ContextProperties &cprops, const bp3d::driver::RenderProperties &rprops, SDL_Window *window)
    : _chain(swap)
    , _device(dev)
    , _deviceContext(devContext)
    , _context(dev, devContext, rprops)
    , _props(cprops)
    , _window(window)
{
    int w;
    int h;
    SDL_GetWindowSize(_window, &w, &h);
    _depthBuffer = _context.GetResourceAllocator().AllocDepthBuffer((bpf::fsize)w, (bpf::fsize)h, bp3d::driver::EDepthBufferFormat::FLOAT_24_STENCIL_8);
    ID3D11Texture2D *buf;
    _chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&buf);
    _context.SetBackBuffer(_depthBuffer, buf);
#ifdef BUILD_DEBUG
    IDXGIDebug1 *dbg;
    DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&dbg);
    dbg->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
#endif
}

DX11StandardDisplay::~DX11StandardDisplay()
{
    _context.GetResourceAllocator().FreeDepthBuffer(_depthBuffer);
    _deviceContext->Release();
    _device->Release();
    _chain->Release();
    SDL_DestroyWindow(_window);
}

void DX11StandardDisplay::Update() noexcept
{
    _chain->Present(0, 0);
}

bool DX11StandardDisplay::PollEvent(bp3d::driver::Event &event) noexcept
{
    SDL_Event ev;
    if (!SDL_PollEvent(&ev))
        return (false);
    if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE)
    {
        event.Type = bp3d::driver::EEventType::DISPLAY_CLOSE;
        return (true);
    }
    //As a test just handle the bare minimum of events
    return (false);
}

void DX11StandardDisplay::SetTitle(const bpf::String &title) noexcept
{
    SDL_SetWindowTitle(_window, *title);
}

void DX11StandardDisplay::SetFullscreen(const bool flag) noexcept
{
    _chain->SetFullscreenState(flag ? TRUE : FALSE, NULL);
}

void DX11StandardDisplay::Resize(const bpf::fsize, const bpf::fsize) noexcept
{
    //Well much harder to implement in DX than GL
}

bpf::math::Matrix4f DX11StandardDisplay::GetPerspectiveProjection(const bpf::math::Viewportf &) const noexcept
{
    return bpf::math::Matrix4f::Identity;
}

bpf::math::Matrix4f DX11StandardDisplay::GetOrthographicProjection(const bpf::math::Viewportf &) const noexcept
{
    return bpf::math::Matrix4f::Identity;
}

bpf::math::Matrix3f DX11StandardDisplay::GetScreenProjection() const noexcept
{
    return bpf::math::Matrix3f::Identity;
}
