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
#include <Framework/Memory/UniquePtr.hpp>
#include <Framework/System/IApplication.hpp>
#include "Engine/Driver/IDisplay.hpp"

namespace bp3d
{
    namespace driver
    {
        enum class BP3D_API ETextureCubeFormat
        {
            /**
             * The target API does not support cube maps
             */
            UNSUPPORTED,

            /**
             * Instructs the caller to store cube maps as an unwrapped cube (the same way DirectX expects it)
             */
            ARRAY_RIGHT_LEFT_FORWARD_BACKWARD_UP_DOWN
        };

        struct BP3D_API RenderEngineProperties
        {
            bool SupportsVR;
            bool SupportsRTX;
            bool FlipTextureSpace;
            bool SupportsRGB; //DX11 is a nightmare as RGB is not supported, this tells the engine client DLL to use RGBA and pre-compile an RGBA texture
            bpf::String DriverName;
            bpf::fint NumScreens;
            ETextureCubeFormat CubeMapFormat;
        };

        class BP3D_API IRenderEngine
        {
        public:
            virtual ~IRenderEngine() {}
            virtual bpf::memory::UniquePtr<IDisplay> CreateDisplay(bpf::system::IApplication &app, const bpf::String &title, const bpf::fsize width, const bpf::fsize height) = 0;
            virtual RenderEngineProperties GetProperties() const noexcept = 0;
        };
    }
}
