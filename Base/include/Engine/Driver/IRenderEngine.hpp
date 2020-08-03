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
#include <Framework/System/Application.hpp>
#include "Engine/Driver/IDisplay.hpp"
#include "Engine/Driver/IShaderCompiler.hpp"

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
            /**
             * Does this renderer supports raytracing
             * Currently all renderers will have false here
             */
            bool SupportsRTX;

            /**
             * Should the caller flip texture space
             * If FlipTextureSpace is true then (0, 0) is the bottom left corner of the texture
             * The default of the caller is expected to be (0, 0) as the top left corner
             */
            bool FlipTextureSpace;

            /**
             * Does this renderer supports RGB textures/render targets
             */
            bool SupportsRGB;

            /**
             * Should we call LockVertexFormat before LockVertexBuffer in order to set the current expected input layout for the shader stage
             * Currently, GL sets this to false as GL combines vertex format and vertex buffer
             */
            bool SeparateVertexFormat;

            /**
             * Typically set to true for DirectX renderers which support setting blend state for a single render target slot
             * GL renderers will have this value set to false and instead the first component inside a blend state will be used for ALL render target slots
             */
            bool SupportsMultiBlending;

            /**
             * Bool set to true if the underlying rendering API uses independent stage bindings
             * Typically set to true for DirectX 11 renderer but set to false for both DirectX 12 and GL
             * Indeed it actually comes to be faster to not use independent stage bindings as this increases root signature cost
             */
            bool IndependentStage;

            /**
             * True if the renderer supports 32 bits pure float (without stencil) as depth buffer format
             */
            bool Supports32FloatDepth;

            /**
             * The name of this render driver (ex: DirectX 11, OpenGL 4.0, ...)
             */
            bpf::String DriverName;

            /**
             * The expected format of the texture buffer for loading a cube map
             */
            ETextureCubeFormat CubeMapFormat;
        };

        struct BP3D_API RenderProperties
        {
            bool VSync;
            bool Antialiasing;
            bool Multisampling;
        };

        struct BP3D_API DisplayMode
        {
            /**
             * Is this display a VR device
             */
            bool IsVR;

            /**
             * Is this display full screen
             */
            bool Fullscreen;

            /**
             * Width of the display
             */
            bpf::fsize Width;

            /**
             * Height of the display
             */
            bpf::fsize Height;

            /**
             * Identifier for the render engine implementation
             */
            bpf::fsize Id;
        };

        class BP3D_API IRenderEngine
        {
        public:
            virtual ~IRenderEngine() {}
            virtual bpf::memory::UniquePtr<IStandardDisplay> CreateStandardDisplay(bpf::system::Application &app, const bpf::String &title, const DisplayMode &mode, const RenderProperties &props) = 0;
            virtual bpf::memory::UniquePtr<IVRDisplay> CreateVRDisplay(bpf::system::Application &app, const DisplayMode &mode, const RenderProperties &props) = 0;
            virtual bpf::memory::UniquePtr<IShaderCompiler> CreateShaderCompiler() = 0;
            virtual bpf::collection::ArrayList<DisplayMode> GetDisplayModes() noexcept = 0;
            virtual const RenderEngineProperties &GetProperties() const noexcept = 0;
        };
    }
}
