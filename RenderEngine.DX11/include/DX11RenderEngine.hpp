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
#include <Engine/Driver/IRenderEngine.hpp>

namespace dx11
{
    class DX11RenderEngine final : public bp3d::driver::IRenderEngine
    {
    private:
        bp3d::driver::RenderEngineProperties _props;
        IDXGIFactory1 *_factory;
        DXGI_ADAPTER_DESC1 _adapterDesc;
        DXGI_MODE_DESC *_modes;
        UINT _modeNum;

    public:
        DX11RenderEngine();
        ~DX11RenderEngine();
        bpf::memory::UniquePtr<bp3d::driver::IStandardDisplay> CreateStandardDisplay(bpf::system::Application &app, const bpf::String &title, const bp3d::driver::DisplayMode &mode, const bp3d::driver::RenderProperties &props);
        bpf::memory::UniquePtr<bp3d::driver::IVRDisplay> CreateVRDisplay(bpf::system::Application &app, const bp3d::driver::DisplayMode &mode, const bp3d::driver::RenderProperties &props);
        bpf::memory::UniquePtr<bp3d::driver::IShaderCompiler> CreateShaderCompiler();
        bpf::collection::ArrayList<bp3d::driver::DisplayMode> GetDisplayModes() noexcept;

        inline const bp3d::driver::RenderEngineProperties &GetProperties() const noexcept
        {
            return (_props);
        }
    };
}
