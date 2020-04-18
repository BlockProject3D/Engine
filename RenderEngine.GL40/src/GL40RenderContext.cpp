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

#include "GL40RenderContext.hpp"

using namespace gl40;

void GL40RenderContext::LockConstantBuffer(bp3d::driver::Resource resource, const bpf::fint reg) noexcept
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint ubo = res.Ptrs[0];
#else
    GLuint ubo = res.Ptr;
#endif
    glBindBufferBase(GL_UNIFORM_BUFFER, reg, ubo);
}

void GL40RenderContext::UpdateConstantBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint ubo = res.Ptrs[0];
#else
    GLuint ubo = res.Ptr;
#endif
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void GL40RenderContext::LockTexture(bp3d::driver::Resource resource, const bpf::fint reg) noexcept
{
    Texture2D *res = reinterpret_cast<Texture2D *>(resource);
    glActiveTexture(GL_TEXTURE0 + reg);
    glBindTexture(res->Target, res->TexId);
}

void GL40RenderContext::UpdateTexture(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept
{
    Texture2D *res = reinterpret_cast<Texture2D *>(resource);
    GLsizei h = (GLsizei)size / res->Width;
    glBindTexture(res->Target, res->TexId);
    glTexSubImage2D(res->Target, 0, 0, 0, res->Width, h, res->Format, res->Type, data);
}

void GL40RenderContext::LockSampler(bp3d::driver::Resource resource, const bpf::fint reg) noexcept
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint sampler = res.Ptrs[0];
#else
    GLuint sampler = res.Ptr;
#endif
    glBindSampler(GL_TEXTURE0 + reg, sampler);
}

void GL40RenderContext::SetRenderTarget(bp3d::driver::Resource resource) noexcept
{
    if (resource == Null)
    {
        _curRT = Null;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint fbo = res.Ptrs[0];
#else
    GLuint fbo = res.Ptr;
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    _curRT = resource;
}

void GL40RenderContext::LockIndexBuffer(bp3d::driver::Resource resource) noexcept
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint ibo = res.Ptrs[0];
#else
    GLuint ibo = res.Ptr;
#endif
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void GL40RenderContext::UpdateIndexBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint ibo = res.Ptrs[0];
#else
    GLuint ibo = res.Ptr;
#endif
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void GL40RenderContext::LockVertexBuffer(bp3d::driver::Resource resource, const bpf::uint32) noexcept
{
#ifdef X86_64
    VertexBuffer vbuf;
    vbuf.Ptr = resource;
    glBindVertexArray(vbuf.Data.VAO);
#else
    VertexBuffer *vb = reinterpret_cast<VertexBuffer *>(resource);
    glBindVertexArray(vb->VAO);
#endif
}

void GL40RenderContext::UpdateVertexBuffer(bp3d::driver::Resource resource, const void *data, const bpf::fsize size) noexcept
{
#ifdef X86_64
    VertexBuffer vbuf;
    vbuf.Ptr = resource;
    glBindBuffer(GL_ARRAY_BUFFER, vbuf.Data.VBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
#else
    VertexBuffer *vb = reinterpret_cast<VertexBuffer *>(resource);
    glBindBuffer(GL_ARRAY_BUFFER, vb->VBO);
#endif
}

void GL40RenderContext::Draw(const bpf::uint32 index, const bpf::uint32 count) noexcept
{
    glDrawArrays(GL_TRIANGLES, index, count);
}

void GL40RenderContext::DrawInstanced(const bpf::uint32 index, const bpf::uint32 count, const bpf::uint32 instanceCount) noexcept
{
    glDrawArraysInstanced(GL_TRIANGLES, index, count, instanceCount);
}

void GL40RenderContext::DrawIndexed(const bpf::uint32 index, const bpf::uint32 count) noexcept
{
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void *)index);
}

void GL40RenderContext::DrawPatches(const bpf::uint32 index, const bpf::uint32 count, const bpf::uint8 controlPoints) noexcept
{
    glPatchParameteri(GL_PATCH_VERTICES, controlPoints);
    glDrawArrays(GL_PATCHES, index, count);
}

void GL40RenderContext::Clear(const bool colorBuffer, const bool depthBuffer) noexcept
{
    GLenum e = 0;
    if (colorBuffer)
        e = GL_COLOR_BUFFER_BIT;
    if (depthBuffer)
        e |= GL_DEPTH_BUFFER_BIT;
    glClear(e);
}

void GL40RenderContext::SetViewport(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept
{
    glViewport(x, y, (GLsizei)w, (GLsizei)h);
}

void GL40RenderContext::SetScissor(const bpf::fint x, bpf::fint y, bpf::fsize w, bpf::fsize h) noexcept
{
    glScissor(x, y, (GLsizei)w, (GLsizei)h);
}

void GL40RenderContext::ReadPixels(void *output, const bpf::fint x, const bpf::fint y, const bpf::fsize w, const bpf::fsize h) noexcept
{
    glReadPixels(x, y, (GLsizei)w, (GLsizei)h, GL_RGBA, GL_UNSIGNED_BYTE, output);
}

void GL40RenderContext::LockPipeline(bp3d::driver::Resource resource) noexcept
{
    if (resource == _curPipeline)
        return;
    if (resource == Null)
    {
        _curPipeline = Null;
        return;
    }
    Pipeline *pipeline = reinterpret_cast<Pipeline *>(resource);
    if (_curPipeline == Null)
    {
        glUseProgram(pipeline->Program);
        if (pipeline->BlendState.Enable)
        {
            glEnable(GL_BLEND);
            glBlendColor(pipeline->BlendState.Factor.X, pipeline->BlendState.Factor.Y, pipeline->BlendState.Factor.Z, pipeline->BlendState.Factor.W);
            glBlendEquationSeparate(pipeline->BlendState.ColorOp, pipeline->BlendState.AlphaOp);
            glBlendFuncSeparate(pipeline->BlendState.SrcColor, pipeline->BlendState.DstColor, pipeline->BlendState.SrcAlpha, pipeline->BlendState.DstAlpha);
        }
        else
            glDisable(GL_BLEND);
        if (pipeline->DepthEnable)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        if (pipeline->ScissorEnable)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
        glDepthMask(pipeline->DepthWriteEnable ? GL_TRUE : GL_FALSE);
        if (pipeline->CullingMode == GL_CULL_FACE)
            glDisable(GL_CULL_FACE);
        else
        {
            glEnable(GL_CULL_FACE);
            glCullFace(pipeline->CullingMode);
        }
        glPolygonMode(GL_FRONT_AND_BACK, pipeline->RenderMode);
    }
    else
    {
        if (_curPipeline->Program != pipeline->Program)
            glUseProgram(pipeline->Program);
        if (_curPipeline->BlendState.HashCode != pipeline->BlendState.HashCode)
        {
            if (pipeline->BlendState.Enable)
            {
                glEnable(GL_BLEND);
                glBlendColor(pipeline->BlendState.Factor.X, pipeline->BlendState.Factor.Y, pipeline->BlendState.Factor.Z, pipeline->BlendState.Factor.W);
                glBlendEquationSeparate(pipeline->BlendState.ColorOp, pipeline->BlendState.AlphaOp);
                glBlendFuncSeparate(pipeline->BlendState.SrcColor, pipeline->BlendState.DstColor, pipeline->BlendState.SrcAlpha, pipeline->BlendState.DstAlpha);
            }
            else
                glDisable(GL_BLEND);
        }
        if (_curPipeline->DepthEnable != pipeline->DepthEnable)
        {
            if (pipeline->DepthEnable)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
        }
        if (_curPipeline->ScissorEnable != pipeline->ScissorEnable)
        {
            if (pipeline->ScissorEnable)
                glEnable(GL_SCISSOR_TEST);
            else
                glDisable(GL_SCISSOR_TEST);
        }
        if (_curPipeline->DepthWriteEnable != pipeline->DepthWriteEnable)
            glDepthMask(pipeline->DepthWriteEnable ? GL_TRUE : GL_FALSE);
        if (_curPipeline->CullingMode != pipeline->CullingMode)
        {
            if (pipeline->CullingMode == GL_CULL_FACE)
                glDisable(GL_CULL_FACE);
            else
            {
                glEnable(GL_CULL_FACE);
                glCullFace(pipeline->CullingMode);
            }
        }
        if (_curPipeline->RenderMode != pipeline->RenderMode)
            glPolygonMode(GL_FRONT_AND_BACK, pipeline->RenderMode);
    }
    _curPipeline = pipeline;
}
