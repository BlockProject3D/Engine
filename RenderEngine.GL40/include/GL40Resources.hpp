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
#include <glad.h>
#include <Engine\Driver\VertexFormatDescriptor.hpp>

namespace gl40
{
#ifdef X86_64
    union ObjectResource
    {
        GLuint Ptrs[2];
        bp3d::driver::Resource Data;
    };

    struct Texture2DInner
    {
        GLuint TexId;
        GLenum Target;
    };

    struct VertexBufferInner
    {
        GLuint VBO;
        GLuint VAO;
    };

    union Texture2D
    {
        Texture2DInner Data;
        bp3d::driver::Resource Ptr;
    };

    union VertexBuffer
    {
        VertexBufferInner Data;
        bp3d::driver::Resource Ptr;
    };
#else
    union ObjectResource
    {
        GLuint Ptr;
        bp3d::driver::Resource Data;
    };
#endif

    struct VertexFormat
    {
        bpf::collection::Array<bp3d::driver::EVertexComponentType> Components;
        GLsizei Stride;
    };

    struct BlendState
    {
        GLenum SrcColor;
        GLenum DstColor;
        GLenum SrcAlpha;
        GLenum DstAlpha;
        GLenum AlphaOp;
        GLenum ColorOp;
        bool Enable;
    };

#ifdef X86
    struct Texture2D
    {
        GLuint TexId;
        GLenum Target;
    };

    struct VertexBuffer
    {
        GLuint VBO;
        GLuint VAO;
    };
#endif
}
