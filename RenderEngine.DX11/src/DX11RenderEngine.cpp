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
#include "DX11RenderEngine.hpp"
#include "DX11Display.hpp"
#include "DX11ShaderCompiler.hpp"
#include <Framework/System/PluginInterface.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#ifdef BUILD_DEBUG
#include <dxgi1_3.h>
#include <dxgidebug.h>
#endif

using namespace dx11;

DX11RenderEngine::DX11RenderEngine()
{
    _props.CubeMapFormat = bp3d::driver::ETextureCubeFormat::ARRAY_RIGHT_LEFT_FORWARD_BACKWARD_UP_DOWN;
    _props.DriverName = "DirectX 11";
    _props.FlipTextureSpace = false;
    _props.SupportsRGB = false;
    _props.SupportsRTX = false;
    _props.SeparateVertexFormat = true;
    _props.SupportsMultiBlending = true;
    _props.IndependentStage = true;
    _props.Supports32FloatDepth = true;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)(&_factory))))
    {
        _factory = Null;
        return;
    }
    IDXGIAdapter1 *adapter;
    if (FAILED(_factory->EnumAdapters1(0, &adapter)))
    {
        _factory->Release();
        _factory = Null;
        return;
    }
    IDXGIOutput *output;
    if (FAILED(adapter->EnumOutputs(0, &output)))
    {
        adapter->Release();
        _factory->Release();
        _factory = Null;
        return;
    }
    UINT numModes;
    if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, Null)))
    {
        output->Release();
        adapter->Release();
        _factory->Release();
        _factory = Null;
        return;
    }
    _modeNum = numModes;
    _modes = new DXGI_MODE_DESC[_modeNum];
    if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, _modes)))
    {
        delete[] _modes;
        output->Release();
        adapter->Release();
        _factory->Release();
        _factory = Null;
        return;
    }
    adapter->GetDesc1(&_adapterDesc);
    output->Release();
    adapter->Release();
    _factory->Release();
}

DX11RenderEngine::~DX11RenderEngine()
{
    if (_factory != Null)
        delete[] _modes;
#ifdef BUILD_DEBUG
    IDXGIDebug1 *dbg;
    DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&dbg);
    dbg->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
#endif
}

bpf::memory::UniquePtr<bp3d::driver::IStandardDisplay> DX11RenderEngine::CreateStandardDisplay(bpf::system::Application &, const bpf::String &title, const bp3d::driver::DisplayMode &mode, const bp3d::driver::RenderProperties &props)
{
    if (mode.IsVR)
        throw bpf::RuntimeException("RenderEngine", "Use CreateVRDisplay to create a VR display");
    if (_factory == Null)
        throw bpf::RuntimeException("RenderEngine", "Could not initialize IDXGIFactory1");
    if (mode.Id > _modeNum)
        throw bpf::RuntimeException("RenderEngine", "Invalid DiplayMode specified");
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    bp3d::driver::ContextProperties cprops;
    cprops.HardwareName = bpf::String::FromUTF16(reinterpret_cast<const bpf::fchar16 *>(_adapterDesc.Description));
    cprops.MaxVRAM = _adapterDesc.DedicatedVideoMemory / 1024 / 1024;
    cprops.SupportsMultiSampling = false;
    cprops.MaxImageQuality = 0;
    cprops.SupportsAnisotropicFiltering = true;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    Uint32 flags = 0;
    if (mode.Fullscreen)
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_Window *window = SDL_CreateWindow(*title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)mode.Width, (int)mode.Height, flags);
    if (window == Null)
        throw bpf::RuntimeException("RenderEngine", "Could not create window");
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;
    swapChainDesc.Windowed = mode.Fullscreen ? FALSE : TRUE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = (UINT)mode.Width;
    swapChainDesc.BufferDesc.Height = (UINT)mode.Height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    if (props.VSync)
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = _modes[mode.Id].RefreshRate.Numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = _modes[mode.Id].RefreshRate.Denominator;
    }
    else
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.Flags = 0;
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    ID3D11Device *device;
    ID3D11DeviceContext *deviceContext;
    IDXGISwapChain *swapChain;
#ifdef BUILD_DEBUG
    UINT flag = D3D11_CREATE_DEVICE_DEBUG;
#else
    UINT flag = 0;
#endif
    if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flag, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &deviceContext)))
    {
        SDL_DestroyWindow(window);
        throw bpf::RuntimeException("RenderEngine", "Could not create ID3D11Device");
    }
    cprops.MaxAnisotropicLevel = D3D11_DEFAULT_MAX_ANISOTROPY;
    cprops.MaxTextureWidth = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    cprops.MaxTextureHeight = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    return (bpf::memory::MakeUnique<DX11StandardDisplay>(swapChain, device, deviceContext, cprops, props, window));
}

bpf::memory::UniquePtr<bp3d::driver::IVRDisplay> DX11RenderEngine::CreateVRDisplay(bpf::system::Application &, const bp3d::driver::DisplayMode &, const bp3d::driver::RenderProperties &)
{
    throw bpf::RuntimeException("RenderEngine", "VR is not yet supported on DirectX11");
}

bpf::memory::UniquePtr<bp3d::driver::IShaderCompiler> DX11RenderEngine::CreateShaderCompiler()
{
    return (bpf::memory::MakeUnique<DX11ShaderCompiler>());
}

bpf::collection::ArrayList<bp3d::driver::DisplayMode> DX11RenderEngine::GetDisplayModes() noexcept
{
    bpf::collection::ArrayList<bp3d::driver::DisplayMode> modes;
    if (_factory == Null)
        return (modes);
    for (UINT i = 0; i != _modeNum; ++i)
    {
        bp3d::driver::DisplayMode md;
        md.Width = _modes[i].Width;
        md.Height = _modes[i].Height;
        md.Id = i;
        md.IsVR = false;
        md.Fullscreen = false;
        modes.Add(md);
        md.Fullscreen = true;
        modes.Add(md);
    }
    return (modes);
}

BP_IMPLEMENT_PLUGIN(RenderEngine, bp3d::driver::IRenderEngine, dx11::DX11RenderEngine);
