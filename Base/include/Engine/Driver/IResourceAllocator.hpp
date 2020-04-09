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
#include "Engine/Driver/ShaderProgramDescriptor.hpp"
#include "Engine/Driver/TextureDescriptor.hpp"
#include "Engine/Driver/SamplerDescriptor.hpp"
#include "Engine/Driver/RenderTargetDescriptor.hpp"
#include "Engine/Driver/BufferDescriptor.hpp"
#include "Engine/Driver/VertexFormatDescriptor.hpp"
#include "Engine/Driver/BlendStateDescriptor.hpp"
#include "Engine/Driver/Resource.hpp"

namespace bp3d
{
    namespace driver
    {
        enum class BP3D_API EDepthBufferFormat
        {
            FLOAT_32_STENCIL_8,
            FLOAT_24_STENCIL_8,
            FLOAT_32
        };

        /**
         * The main allocator class for GPU resources
         * Usually the use of raw pointers and Free* style of functions is bad in C++ but here we have no other ways
         * if we want to support fast abstraction to the render engine.
         * Another consern is DirectX 12 and Vulkan: in DX12 and Vulkan you cannot immediatly free the memory of a resource as this would
         * crash the entire renderer if the GPU is currently using it; the free has to be deffered to correspond at the time the GPU
         * no longer uses the resource.
         */
        class BP3D_API IResourceAllocator
        {
        public:
            virtual ~IResourceAllocator() {}
            virtual Resource AllocDepthBuffer(const bpf::fsize width, const bpf::fsize height, const EDepthBufferFormat format) = 0;
            virtual Resource AllocTexture2D(const EBufferType type, const TextureDescriptor &descriptor) = 0;
            virtual Resource AllocTexture2DArray(const EBufferType type, const TextureDescriptor &descriptor, const bpf::fsize layers) = 0;
            virtual Resource AllocTextureCube(const EBufferType type, const TextureDescriptor &descriptor) = 0;
            virtual Resource AllocSampler(const SamplerDescriptor &descriptor) = 0;
            virtual Resource AllocRenderTargetComponent(const RenderTargetComponentDescriptor &descriptor) = 0;
            virtual Resource AllocRenderTarget(const RenderTargetDescriptor &descriptor) = 0;
            virtual Resource AllocConstantBuffer(const EBufferType type, const BufferDescriptor &descriptor) = 0;
            virtual Resource AllocVertexFormat(const VertexFormatDescriptor &vformat) = 0;
            virtual Resource AllocVertexBuffer(const EBufferType type, Resource vformat, const BufferDescriptor &buffer) = 0;
            virtual Resource AllocIndexBuffer(const EBufferType type, const BufferDescriptor &descriptor) = 0;
            virtual Resource AllocShaderProgram(const ShaderProgramDescriptor &descriptor) = 0;
            virtual Resource AllocBlendState(const BlendStateDescriptor &descriptor) = 0;

            virtual void FreeBlendState(Resource resource) = 0;

            virtual void FreeVertexFormat(Resource resource) = 0;

            virtual void FreeDepthBuffer(Resource resource) = 0;

            /**
             * Request free of a 2D texture
             * @param resource the resource to free
             */
            virtual void FreeTexture2D(Resource resource) = 0;

            /**
             * Request free of a 2D texture array
             * @param resource the resource to free
             */
            virtual void FreeTexture2DArray(Resource resource) = 0;

            /**
             * Request free of a cube map
             * @param resource the resource to free
             */
            virtual void FreeTextureCube(Resource resource) = 0;

            /**
             * Request free of a sampler object
             * @param resource the resource to free
             */
            virtual void FreeSampler(Resource resource) = 0;

            /**
             * Request free of a single render target component
             * @param resource the resource to free
             */
            virtual void FreeRenderTargetComponent(Resource resource) = 0;

            /**
             * Request free of a render target
             * WARNING: Free all components of a render target before freeing that render target, failure to do so is undefined behaviour
             * @param resource the resource to free
             */
            virtual void FreeRenderTarget(Resource resource) = 0;

            /**
             * Request free of a constant buffer
             * @param resource the resource to free
             */
            virtual void FreeConstantBuffer(Resource resource) = 0;

            /**
             * Request free of a vertex buffer
             * @param resource the resource to free
             */
            virtual void FreeVertexBuffer(Resource resource) = 0;

            /**
             * Request free of an index buffer
             * @param resource the resource to free
             */
            virtual void FreeIndexBuffer(Resource resource) = 0;

            /**
             * Request free of a shader program
             * @param resource the resource to free
             */
            virtual void FreeShaderProgram(Resource resource) = 0;
        };
    }
}
