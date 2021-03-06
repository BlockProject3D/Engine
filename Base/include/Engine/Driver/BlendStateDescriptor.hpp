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
#include <Framework/Collection/ArrayList.hpp>
#include <Framework/Math/Vector.hpp>

namespace bp3d
{
    namespace driver
    {
        enum class BP3D_API EBlendOp
        {
            ADD,
            SUBTRACT,
            INVERSE_SUBTRACT,
            MIN,
            MAX
        };

        enum class BP3D_API EBlendFactor
        {
            ZERO,
            ONE,
            SRC_COLOR,
            ONE_MINUS_SRC_COLOR,
            SRC_ALPHA,
            ONE_MINUS_SRC_ALPHA,
            DST_COLOR,
            ONE_MINUS_DST_COLOR,
            DST_ALPHA,
            ONE_MINUS_DST_ALPHA,
            SRC_ALPHA_SATURATE,
            SRC1_COLOR,
            ONE_MINUS_SRC1_COLOR,
            SRC1_ALPHA,
            ONE_MINUS_SRC1_ALPHA
        };

        struct BP3D_API RenderTargetComponentBlendState
        {
            bool Enable;
            EBlendFactor SrcColor;
            EBlendFactor DstColor;
            EBlendFactor SrcAlpha;
            EBlendFactor DstAlpha;
            EBlendOp ColorOp;
            EBlendOp AlphaOp;
        };

        struct BP3D_API BlendStateDescriptor
        {
            bpf::collection::ArrayList<RenderTargetComponentBlendState> Components;
            bpf::math::Vector4f Factor;
        };
    }
}
