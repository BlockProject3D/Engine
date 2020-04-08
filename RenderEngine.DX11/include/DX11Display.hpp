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
#include <Engine/Driver/IDisplay.hpp>
#include <SDL2/SDL.h>
#include "DX11RenderContext.hpp"

namespace dx11
{
    class DX11StandardDisplay final : public bp3d::driver::IStandardDisplay
    {
    private:
        IDXGISwapChain *_chain;
        ID3D11Device *_device;
        ID3D11DeviceContext *_deviceContext;
        DX11RenderContext _context;
        bp3d::driver::ContextProperties _props;
        SDL_Window *_window;
        bp3d::driver::Resource _depthBuffer;

    public:
        DX11StandardDisplay(IDXGISwapChain *swap, ID3D11Device *dev, ID3D11DeviceContext *devContext, const bp3d::driver::ContextProperties &cprops, const bp3d::driver::RenderProperties &rprops, SDL_Window *window);
        ~DX11StandardDisplay();

        inline bp3d::driver::IRenderContext &GetContext() noexcept
        {
            return (_context);
        }

        inline bp3d::driver::ContextProperties GetContextProperties() const noexcept
        {
            return (_props);
        }

        void Update() noexcept;
        bool PollEvent(bp3d::driver::Event &event) noexcept;
        void SetTitle(const bpf::String &title) noexcept;
        void SetFullscreen(const bool flag) noexcept;
        void Resize(const bpf::fsize width, const bpf::fsize height) noexcept;
        bpf::math::Matrix4f GetPerspectiveProjection(const bpf::math::Viewportf &viewport) const noexcept;
        bpf::math::Matrix4f GetOrthographicProjection(const bpf::math::Viewportf &viewport) const noexcept;
        bpf::math::Matrix3f GetScreenProjection() const noexcept;
    };
}
