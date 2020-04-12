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
#include <Framework/Math/Vector.hpp>
#include "Engine/Driver/IResourceAllocator.hpp"

namespace bp3d
{
    namespace driver
    {
        class BP3D_API IRenderContext
        {
        public:
            virtual ~IRenderContext() {}
            virtual IResourceAllocator &GetResourceAllocator() noexcept = 0;

            /**
             * Note: the stage argument is only usefull for DirectX applications
             */
            virtual void LockConstantBuffer(Resource resource, const bpf::fint reg) noexcept = 0;
            virtual void UpdateConstantBuffer(Resource resource, const void *data, const bpf::fsize size) noexcept = 0;
            virtual void LockTexture(Resource resource, const bpf::fint reg) noexcept = 0;
            virtual void UpdateTexture(Resource resource, const void *data, const bpf::fsize size) noexcept = 0;
            virtual void LockSampler(Resource resource, const bpf::fint reg) noexcept = 0;
            virtual void SetRenderTarget(Resource resource) noexcept = 0;
            virtual Resource GetRenderTarget() noexcept = 0;
            virtual void LockIndexBuffer(Resource resource) noexcept = 0;
            virtual void UpdateIndexBuffer(Resource resource, const void *data, const bpf::fsize size) noexcept = 0;
            virtual void LockVertexBuffer(Resource resource, const bpf::uint32 vertexSize) noexcept = 0;
            virtual void UpdateVertexBuffer(Resource resource, const void *data, const bpf::fsize size) noexcept = 0;
            virtual void LockPipeline(Resource resource) noexcept = 0;
            virtual void DrawPatches(const bpf::uint32 index, const bpf::uint32 count, const bpf::uint8 controlPoints) noexcept = 0;
            virtual void Draw(const bpf::uint32 index, const bpf::uint32 count) noexcept = 0;
            virtual void DrawInstanced(const bpf::uint32 index, const bpf::uint32 count, const bpf::uint32 instanceCount) noexcept = 0;
            virtual void DrawIndexed(const bpf::uint32 index, const bpf::uint32 count) noexcept = 0;
            virtual void Clear(const bool colorBuffer = false, const bool depthBuffer = true) noexcept = 0;
            virtual bool ReadPixels(void *output, const bpf::fint x, const bpf::fint y, const bpf::fsize w, const bpf::fsize h) noexcept = 0;
            virtual void SetViewport(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept = 0;
            virtual void SetScissor(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept = 0;
        };
    }
}
