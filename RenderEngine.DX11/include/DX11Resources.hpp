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

/**
 * Here the use of structures is to allow the compiler to perform certain optimizations as those structs are pure C compatible types
 */
namespace dx11
{
    struct Texture2D
    {
        ID3D11ShaderResourceView *View;
        ID3D11Texture2D *Texture;
        DXGI_FORMAT Format;
    };

    struct RenderTarget
    {
        ID3D11RenderTargetView *RTView[8];
        ID3D11DepthStencilView *DepthView;
        UINT RTCount;
    };

    struct DepthBuffer
    {
        ID3D11DepthStencilView *View;
        ID3D11Texture2D *Texture;
    };

    struct ShaderProgram
    {
        int StageFlagsSamplers[16];
        int StageFlagsTextures[16];
        int StageFlagsCBuffers[16];
        ID3D11VertexShader *Vertex;
        ID3D11GeometryShader *Geometry;
        ID3D11PixelShader *Pixel;
        ID3D11HullShader *Hull;
        ID3D11DomainShader *Domain;
    };

    struct BlendState
    {
        ID3D11BlendState *BlendState;
        bpf::math::Vector4f Factor;
    };

    struct Pipeline
    {
        ShaderProgram Program;
        ID3D11RasterizerState *Rasterizer;
        ID3D11BlendState *BlendState;
        ID3D11DepthStencilState *DepthState;
        ID3D11InputLayout *VFormat;
        bpf::math::Vector4f BlendFactor;
    };
}
