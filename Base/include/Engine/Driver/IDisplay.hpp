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
#include <Framework/String.hpp>
#include <Framework/Math/Viewport.hpp>
#include "Engine/Driver/IRenderContext.hpp"
#include "Engine/Driver/Event.hpp"

namespace bp3d
{
    namespace driver
    {
        constexpr bpf::fint SUPPORTS_PERSPECTIVE_PROJECTION = 0x1;
        constexpr bpf::fint SUPPORTS_ORTHOGRAPHIC_PROJECTION = 0x2;
        constexpr bpf::fint SUPPORTS_SCREEN_PROJECTION = 0x4;
        constexpr bpf::fint HAS_VIEW_TRANSFORM = 0x8;

        struct BP3D_API ContextProperties
        {
            bpf::fsize MaxTextureWidth;
            bpf::fsize MaxTextureHeight;
            bpf::String HardwareName;
            bool SupportsMultiSampling;
            bpf::uint32 MaxImageQuality;
            bool SupportsAnisotropicFiltering;
            bpf::uint32 MaxAnisotropicLevel;
        };

        class BP3D_API IDisplay
        {
        public:
            virtual ~IDisplay() {}
            virtual IRenderContext &GetContext() noexcept = 0;
            virtual void SetTitle(const bpf::String &title) noexcept = 0;
            virtual void SetFullscreen(const bool flag) noexcept = 0;
            virtual void Resize(const bpf::fsize width, const bpf::fsize height) noexcept = 0;
            virtual bpf::math::Matrix4f GetPerspectiveProjection(const bpf::math::Viewportf &viewport, const bpf::fint screen) const noexcept = 0;
            virtual bpf::math::Matrix4f GetOrthographicProjection(const bpf::math::Viewportf &viewport, const bpf::fint screen) const noexcept = 0;
            virtual bpf::math::Matrix3f GetScreenProjection(const bpf::fint screen) const noexcept = 0;
            virtual bpf::math::Matrix4f GetViewTransformMatrix(const bpf::fint screen) noexcept = 0;
            virtual bpf::fint GetScreenProperties(const bpf::fint screen) const noexcept = 0;
            virtual ContextProperties GetContextProperties() const noexcept = 0;
            virtual void Update() noexcept = 0;
            virtual bool PollEvent(Event &event) noexcept = 0;
        };
    }
}
