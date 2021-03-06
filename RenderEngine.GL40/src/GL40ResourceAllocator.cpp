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

#include <Framework/RuntimeException.hpp>
#include "GL40ResourceAllocator.hpp"
#include "GL40Resources.hpp"
#include "ShaderDecoder.hpp"

using namespace gl40;

void GL40ResourceAllocator::SetupTextureFormat(const bp3d::driver::TextureDescriptor &descriptor, GLenum &internalFormat, GLenum &format, GLenum &t, GLsizei &slicemempitch)
{
    switch (descriptor.Format)
    {
    case bp3d::driver::ETextureFormat::R_UINT_8:
        internalFormat = GL_RED;
        format = GL_RED;
        t = GL_UNSIGNED_BYTE;
        slicemempitch = (GLsizei)descriptor.Width * (GLsizei)descriptor.Height;
        break;
    case bp3d::driver::ETextureFormat::R_FLOAT_32:
        internalFormat = GL_R32F;
        format = GL_RED;
        t = GL_FLOAT;
        slicemempitch = (GLsizei)descriptor.Width * (GLsizei)descriptor.Height * 4;
        break;
    case bp3d::driver::ETextureFormat::RGB_UINT_8:
        internalFormat = GL_RGB;
        format = GL_RGB;
        t = GL_UNSIGNED_BYTE;
        slicemempitch = (GLsizei)descriptor.Width * (GLsizei)descriptor.Height * 3;
        break;
    case bp3d::driver::ETextureFormat::RGBA_UINT_8:
        internalFormat = GL_RGBA;
        format = GL_RGBA;
        t = GL_UNSIGNED_BYTE;
        slicemempitch = (GLsizei)descriptor.Width * (GLsizei)descriptor.Height * 4;
        break;
    }
    if (descriptor.Compression != bp3d::driver::ETextureCompression::NONE)
    {
        switch (descriptor.Compression)
        {
        case bp3d::driver::ETextureCompression::BC1_OR_DXT1:
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            break;
        case bp3d::driver::ETextureCompression::BC2_OR_DXT3:
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            break;
        case bp3d::driver::ETextureCompression::BC3_OR_DXT5:
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        }
    }
}

bp3d::driver::Resource GL40ResourceAllocator::AllocTexture2D(const bp3d::driver::EBufferType, const bp3d::driver::TextureDescriptor &descriptor)
{
    GLenum internalFormat;
    GLenum format;
    GLenum t;
    GLuint tex;
    GLsizei slicemempitch;
    SetupTextureFormat(descriptor, internalFormat, format, t, slicemempitch);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    //Force GL to use Nearest by default (even if we're gonna use sampler objects)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //Now load the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, (GLsizei)descriptor.Width, (GLsizei)descriptor.Height, 0, format, t, descriptor.Data);
    if (descriptor.MipMaps > 1)
        glGenerateMipmap(GL_TEXTURE_2D);
    Texture2D *res = static_cast<Texture2D *>(bpf::memory::Memory::Malloc(sizeof(Texture2D)));
    res->Target = GL_TEXTURE_2D;
    res->TexId = tex;
    res->Width = (GLsizei)descriptor.Width;
    res->Format = format;
    res->Type = t;
    return (res);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocTexture2DArray(const bp3d::driver::EBufferType, const bp3d::driver::TextureDescriptor &descriptor, const bpf::fsize layers)
{
    GLenum internalFormat;
    GLenum format;
    GLenum t;
    GLuint tex;
    GLsizei slicemempitch;
    SetupTextureFormat(descriptor, internalFormat, format, t, slicemempitch);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, descriptor.MipMaps, internalFormat, (GLsizei)descriptor.Width, (GLsizei)descriptor.Height, (GLsizei)layers);
    //Force GL to use Nearest by default (even if we're gonna use sampler objects)
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    bpf::uint8 *ptr = reinterpret_cast<bpf::uint8 *>(descriptor.Data);
    for (bpf::fsize i = 0; i != layers; ++i)
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)i, (GLsizei)descriptor.Width, (GLsizei)descriptor.Height, 1, format, t, ptr + (i * slicemempitch));
    if (descriptor.MipMaps > 1)
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    Texture2D *res = static_cast<Texture2D *>(bpf::memory::Memory::Malloc(sizeof(Texture2D)));
    res->Target = GL_TEXTURE_2D_ARRAY;
    res->TexId = tex;
    res->Width = (GLsizei)descriptor.Width;
    res->Format = format;
    res->Type = t;
    return (res);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocTextureCube(const bp3d::driver::EBufferType, const bp3d::driver::TextureDescriptor &descriptor)
{
    GLenum internalFormat;
    GLenum format;
    GLenum t;
    GLuint tex;
    GLsizei slicemempitch;
    SetupTextureFormat(descriptor, internalFormat, format, t, slicemempitch);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    //Force GL to use Nearest by default (even if we're gonna use sampler objects)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    bpf::uint8 *ptr = reinterpret_cast<bpf::uint8 *>(descriptor.Data);
    for (GLuint i = 0; i != 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, (GLsizei)descriptor.Width, (GLsizei)descriptor.Height, 0, format, t, ptr + (i * slicemempitch));
    if (descriptor.MipMaps > 1)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    Texture2D *res = static_cast<Texture2D *>(bpf::memory::Memory::Malloc(sizeof(Texture2D)));
    res->Target = GL_TEXTURE_CUBE_MAP;
    res->TexId = tex;
    res->Width = (GLsizei)descriptor.Width;
    res->Format = format;
    res->Type = t;
    return (res);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocSampler(const bp3d::driver::SamplerDescriptor &descriptor)
{
    GLuint sampler;
    glGenSamplers(1, &sampler);
    switch (descriptor.FilterFunc)
    {
    case bp3d::driver::ETextureFiltering::MIN_MAG_LINEAR_MIPMAP_LINEAR:
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        break;
    case bp3d::driver::ETextureFiltering::MIN_MAG_LINEAR_MIPMAP_POINT:
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        break;
    case bp3d::driver::ETextureFiltering::MIN_MAG_POINT_MIPMAP_LINEAR:
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        break;
    case bp3d::driver::ETextureFiltering::MIN_MAG_POINT_MIPMAP_POINT:
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        break;
    }
    //TODO: Setup AddressModeU/V/W
    ObjectResource res;
#ifdef X86_64
    res.Ptrs[0] = sampler;
#else
    res.Ptr = sampler;
#endif
    return (res.Data);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocRenderTargetComponent(const bp3d::driver::RenderTargetComponentDescriptor &descriptor)
{
    GLenum internalFormat;
    GLenum format;
    GLenum t;
    GLuint tex;
    GLsizei slicemempitch;
    SetupTextureFormat(descriptor.Texture, internalFormat, format, t, slicemempitch);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    //Force GL to use Nearest by default (even if we're gonna use sampler objects)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, (GLsizei)descriptor.Texture.Width, (GLsizei)descriptor.Texture.Height, 0, format, t, Null);
    Texture2D *res = static_cast<Texture2D *>(bpf::memory::Memory::Malloc(sizeof(Texture2D)));
    res->Target = GL_TEXTURE_2D;
    res->TexId = tex;
    res->Width = (GLsizei)descriptor.Texture.Width;
    res->Format = format;
    res->Type = t;
    return (res);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocDepthBuffer(const bpf::fsize width, const bpf::fsize height, const bp3d::driver::EDepthBufferFormat format)
{
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    if (format == bp3d::driver::EDepthBufferFormat::FLOAT_32_STENCIL_8)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, (GLsizei)width, (GLsizei)height);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)width, (GLsizei)height);
    ObjectResource res;
#ifdef X86_64
    res.Ptrs[0] = rbo;
#else
    res.Ptr = rbo;
#endif
    return (res.Data);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocRenderTarget(const bp3d::driver::RenderTargetDescriptor &descriptor)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw bpf::RuntimeException("RenderEngine", "glGenFramebuffers failed");
    bpf::collection::Array<GLuint> textures(descriptor.Components.Size());
    for (bpf::fsize i = 0; i != descriptor.Components.Size(); ++i)
    {
        Texture2D *res = reinterpret_cast<Texture2D *>(descriptor.Components[i]);
        GLuint texture = res->TexId;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (GLenum)i, GL_TEXTURE_2D, texture, 0);
        textures[i] = texture;
    }
    if (descriptor.DepthBuffer != Null)
    {
        ObjectResource res;
        res.Data = descriptor.DepthBuffer;
#ifdef X86_64
        GLuint rbo = res.Ptrs[0];
#else
        GLuint rbo = res.Ptr;
#endif
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    }
    glDrawBuffers((GLsizei)textures.Size(), *textures);
    ObjectResource res;
#ifdef X86_64
    res.Ptrs[0] = fbo;
#else
    res.Ptr = fbo;
#endif
    return (res.Data);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocConstantBuffer(const bp3d::driver::EBufferType type, const bp3d::driver::BufferDescriptor &descriptor)
{
    GLenum t = 0;
    GLuint ubo;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        t = GL_DYNAMIC_DRAW;
        break;
    case bp3d::driver::EBufferType::STATIC:
        t = GL_STATIC_DRAW;
        break;
    }
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, descriptor.Size, descriptor.Data, t);
    ObjectResource res;
#ifdef X86_64
    res.Ptrs[0] = ubo;
#else
    res.Ptr = ubo;
#endif
    return (res.Data);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocFixedConstantBuffer(const bp3d::driver::EBufferType type, const int reg, const bp3d::driver::BufferDescriptor &descriptor)
{
    GLenum t = 0;
    GLuint ubo;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        t = GL_DYNAMIC_DRAW;
        break;
    case bp3d::driver::EBufferType::STATIC:
        t = GL_STATIC_DRAW;
        break;
    }
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, descriptor.Size, descriptor.Data, t);
    ObjectResource res;
#ifdef X86_64
    res.Ptrs[0] = ubo;
#else
    res.Ptr = ubo;
#endif
    if ((reg + 1) > _fixedConstBufs.Size())
        _fixedConstBufs.Resize(reg + 1);
    _fixedConstBufs[reg] = ubo;
    return (res.Data);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocVertexFormat(const bp3d::driver::VertexFormatDescriptor &descriptor)
{
    VertexFormat *vf = bpf::memory::MemUtils::New<VertexFormat>();
    vf->Components = bpf::collection::Array<bp3d::driver::EVertexComponentType>(descriptor.Components.Size());
    GLsizei offset = 0;
    for (bpf::fsize i = 0; i != descriptor.Components.Size(); ++i)
    {
        vf->Components[i] = descriptor.Components[i].Type;
        switch (vf->Components[i])
        {
        case bp3d::driver::EVertexComponentType::FLOAT:
            offset += sizeof(GLfloat);
            break;
        case bp3d::driver::EVertexComponentType::INT:
            offset += sizeof(GLint);
            break;
        case bp3d::driver::EVertexComponentType::UINT:
            offset += sizeof(GLuint);
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_2:
            offset += sizeof(GLfloat) * 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_3:
            offset += sizeof(GLfloat) * 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_4:
            offset += sizeof(GLfloat) * 4;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_2:
            offset += sizeof(GLint) * 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_3:
            offset += sizeof(GLint) * 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_4:
            offset += sizeof(GLint) * 4;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_2:
            offset += sizeof(GLuint) * 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_3:
            offset += sizeof(GLuint) * 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_4:
            offset += sizeof(GLuint) * 4;
            break;
        }
    }
    vf->Stride = offset;
    return (vf);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocVertexBuffer(const bp3d::driver::EBufferType type, bp3d::driver::Resource vformat, const bp3d::driver::BufferDescriptor &buffer)
{
    VertexFormat *vf = reinterpret_cast<VertexFormat *>(vformat);
    GLuint vao;
    GLuint vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLenum usage = 0;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        usage = GL_DYNAMIC_DRAW;
        break;
    case bp3d::driver::EBufferType::STATIC:
        usage = GL_STATIC_DRAW;
        break;
    }
    glBufferData(GL_ARRAY_BUFFER, buffer.Size, buffer.Data, usage);
    GLsizei offset = 0;
    for (bpf::fsize i = 0; i != vf->Components.Size(); ++i)
    {
        GLenum t = 0;
        GLsizei o = 0;
        GLint size = 0;
        switch (vf->Components[i])
        {
        case bp3d::driver::EVertexComponentType::FLOAT:
            t = GL_FLOAT;
            o = sizeof(GLfloat);
            size = 1;
            break;
        case bp3d::driver::EVertexComponentType::UINT:
            t = GL_UNSIGNED_INT;
            o = sizeof(GLuint);
            size = 1;
            break;
        case bp3d::driver::EVertexComponentType::INT:
            t = GL_INT;
            o = sizeof(GLint);
            size = 1;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_2:
            t = GL_FLOAT;
            o = sizeof(GLfloat) * 2;
            size = 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_3:
            t = GL_FLOAT;
            o = sizeof(GLfloat) * 3;
            size = 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_FLOAT_4:
            t = GL_FLOAT;
            o = sizeof(GLfloat) * 4;
            size = 4;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_2:
            t = GL_INT;
            o = sizeof(GLint) * 2;
            size = 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_3:
            t = GL_INT;
            o = sizeof(GLint) * 3;
            size = 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_INT_4:
            t = GL_INT;
            o = sizeof(GLint) * 4;
            size = 4;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_2:
            t = GL_UNSIGNED_INT;
            o = sizeof(GLuint) * 2;
            size = 2;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_3:
            t = GL_UNSIGNED_INT;
            o = sizeof(GLuint) * 3;
            size = 3;
            break;
        case bp3d::driver::EVertexComponentType::VECTOR_UINT_4:
            t = GL_UNSIGNED_INT;
            o = sizeof(GLuint) * 4;
            size = 4;
            break;
        }
        if (t == GL_INT || GL_UNSIGNED_INT)
            glVertexAttribIPointer((GLuint)i, size, t, vf->Stride, (GLvoid *)offset);
        else
            glVertexAttribPointer((GLuint)i, size, t, GL_FALSE, vf->Stride, (GLvoid *)offset);
        glEnableVertexAttribArray((GLuint)i);
        offset += o;
    }
    glBindVertexArray(0);
#ifdef X86_64
    VertexBuffer res;
    res.Data.VAO = vao;
    res.Data.VBO = vbo;
    return (res.Ptr);
#else
    VertexBuffer *buf = static_cast<VertexBuffer *>(bpf::memory::Memory::Malloc(sizeof(VertexBuffer)));
    buf->VAO = vao;
    buf->VBO = vbo;
    return (buf);
#endif
}

bp3d::driver::Resource GL40ResourceAllocator::AllocIndexBuffer(const bp3d::driver::EBufferType type, const bp3d::driver::BufferDescriptor &descriptor)
{
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    GLenum usage = 0;
    switch (type)
    {
    case bp3d::driver::EBufferType::DYNAMIC:
        usage = GL_DYNAMIC_DRAW;
        break;
    case bp3d::driver::EBufferType::STATIC:
        usage = GL_STATIC_DRAW;
        break;
    }
    glBufferData(GL_ARRAY_BUFFER, descriptor.Size, descriptor.Data, usage);
    ObjectResource res;
#ifdef X86_64
    res.Ptrs[0] = ibo;
#else
    res.Ptr = ibo;
#endif
    return (res.Data);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocShaderProgram(const bp3d::driver::ShaderProgramDescriptor &descriptor)
{
    bpf::collection::ArrayList<BPGLSLUniform> uniforms;
    GLuint shaders[3];
    GLint i = 0;
    for (auto &shader : descriptor.Shaders)
    {
        GLenum type = 0;
        switch (shader.Type)
        {
        case bp3d::driver::EShaderType::GEOMETRY:
            type = GL_GEOMETRY_SHADER;
            break;
        case bp3d::driver::EShaderType::PIXEL:
            type = GL_FRAGMENT_SHADER;
            break;
        case bp3d::driver::EShaderType::VERTEX:
            type = GL_VERTEX_SHADER;
            break;
        }
        GLuint shaderId = glCreateShader(type);
        ShaderDecoder decoder(shader.Data, shader.Size);
        const GLchar *arr = decoder.GetShaderCodePtr();
        const GLint size = (GLint)decoder.GetShaderCodeSize();
        glShaderSource(shaderId, 1, &arr, &size);
        glCompileShader(shaderId);
        GLint success;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        GLchar infoLog[512];
        if (!success)
        {
            for (GLint j = 0; j != i; ++j)
                glDeleteShader(shaders[j]);
            glGetShaderInfoLog(shaderId, 512, Null, infoLog);
            throw bpf::RuntimeException("RenderEngine", bpf::String("Error compiling shader: ") + infoLog);
        }
        shaders[i++] = shaderId;
        BPGLSLUniform uniform;
        while (decoder.GetNextUniform(uniform))
            uniforms.Add(uniform);
    }
    GLuint prog = glCreateProgram();
    for (GLint j = 0; j != i; ++j)
        glAttachShader(prog, shaders[j]);
    glLinkProgram(prog);
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    GLchar infoLog[512];
    if (!success)
    {
        glGetShaderInfoLog(prog, 512, Null, infoLog);
        throw bpf::RuntimeException("RenderEngine", bpf::String("Error linking shader program: ") + infoLog);
    }
    glUseProgram(prog);
    for (auto &uniform : uniforms)
    {
        auto loc = glGetUniformLocation(prog, uniform.Name);
        glUniform1i(loc, (GLint)uniform.Register);
    }
    for (auto &bind : descriptor.Bindings)
    {
        if (bind.Type == bp3d::driver::EBindingType::FIXED_CONSTANT_BUFFER)
            glBindBufferBase(GL_UNIFORM_BUFFER, bind.Register, _fixedConstBufs[bind.Register]);
    }
    for (GLint j = 0; j != i; ++j)
        glDeleteShader(shaders[j]);
    ObjectResource res;
#ifdef X86_64
    res.Ptrs[0] = prog;
#else
    res.Ptr = prog;
#endif
    return (res.Data);
}

GLenum GL40ResourceAllocator::TranslateBlendOp(const bp3d::driver::EBlendOp op)
{
    switch (op)
    {
    case bp3d::driver::EBlendOp::ADD:
        return (GL_FUNC_ADD);
    case bp3d::driver::EBlendOp::SUBTRACT:
        return (GL_FUNC_SUBTRACT);
    case bp3d::driver::EBlendOp::INVERSE_SUBTRACT:
        return (GL_FUNC_REVERSE_SUBTRACT);
    case bp3d::driver::EBlendOp::MIN:
        return (GL_MIN);
    case bp3d::driver::EBlendOp::MAX:
        return (GL_MAX);
    }
    return (GL_FUNC_ADD);
}

GLenum GL40ResourceAllocator::TranslateBlendFactor(const bp3d::driver::EBlendFactor factor)
{
    switch (factor)
    {
    case bp3d::driver::EBlendFactor::ZERO:
        return (GL_ZERO);
    case bp3d::driver::EBlendFactor::ONE:
        return (GL_ONE);
    case bp3d::driver::EBlendFactor::DST_ALPHA:
        return (GL_DST_ALPHA);
    case bp3d::driver::EBlendFactor::DST_COLOR:
        return (GL_DST_COLOR);
    case bp3d::driver::EBlendFactor::ONE_MINUS_DST_ALPHA:
        return (GL_ONE_MINUS_DST_ALPHA);
    case bp3d::driver::EBlendFactor::ONE_MINUS_DST_COLOR:
        return (GL_ONE_MINUS_DST_COLOR);
    case bp3d::driver::EBlendFactor::ONE_MINUS_SRC_ALPHA:
        return (GL_ONE_MINUS_SRC_ALPHA);
    case bp3d::driver::EBlendFactor::ONE_MINUS_SRC_COLOR:
        return (GL_ONE_MINUS_SRC_COLOR);
    case bp3d::driver::EBlendFactor::ONE_MINUS_SRC1_ALPHA:
        return (GL_ONE_MINUS_SRC1_ALPHA);
    case bp3d::driver::EBlendFactor::ONE_MINUS_SRC1_COLOR:
        return (GL_ONE_MINUS_SRC1_COLOR);
    case bp3d::driver::EBlendFactor::SRC1_COLOR:
        return (GL_SRC1_COLOR);
    case bp3d::driver::EBlendFactor::SRC_ALPHA_SATURATE:
        return (GL_SRC_ALPHA_SATURATE);
    case bp3d::driver::EBlendFactor::SRC1_ALPHA:
        return (GL_SRC1_ALPHA);
    case bp3d::driver::EBlendFactor::SRC_COLOR:
        return (GL_SRC_COLOR);
    case bp3d::driver::EBlendFactor::SRC_ALPHA:
        return (GL_SRC_ALPHA);
    }
    return (GL_ZERO);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocBlendState(const bp3d::driver::BlendStateDescriptor &descriptor)
{
    if (descriptor.Components.Size() == 0)
        throw bpf::RuntimeException("RenderEngine", "Can't allocate a Null blend state");
    auto target = descriptor.Components[0];
    BlendState *state = static_cast<BlendState *>(bpf::memory::Memory::Malloc(sizeof(BlendState)));
    state->Enable = target.Enable;
    state->ColorOp = TranslateBlendOp(target.ColorOp);
    state->AlphaOp = TranslateBlendOp(target.AlphaOp);
    state->SrcColor = TranslateBlendFactor(target.SrcColor);
    state->DstColor = TranslateBlendFactor(target.DstColor);
    state->SrcAlpha = TranslateBlendFactor(target.SrcAlpha);
    state->DstAlpha = TranslateBlendFactor(target.DstAlpha);
    state->Factor = descriptor.Factor;
    state->HashCode = (bpf::uintptr)state;
    return (state);
}

bp3d::driver::Resource GL40ResourceAllocator::AllocPipeline(const bp3d::driver::PipelineDescriptor &descriptor)
{
    Pipeline *pipeline = static_cast<Pipeline *>(bpf::memory::Memory::Malloc(sizeof(Pipeline)));
    BlendState *state = reinterpret_cast<BlendState *>(descriptor.BlendState);
    ObjectResource prog;
    prog.Data = descriptor.ShaderProgram;
#ifdef X86_64
    GLuint prg = prog.Ptrs[0];
#else
    GLuint prg = prog.Ptr;
#endif
    if (state == Null)
        pipeline->BlendState.Enable = false;
    else
        pipeline->BlendState = *state;
    pipeline->Program = prg;
    pipeline->DepthWriteEnable = descriptor.DepthWriteEnable;
    pipeline->DepthEnable = descriptor.DepthEnable;
    switch (descriptor.CullingMode)
    {
    case bp3d::driver::ECullingMode::BACK_FACE:
        pipeline->CullingMode = GL_BACK;
        break;
    case bp3d::driver::ECullingMode::FRONT_FACE:
        pipeline->CullingMode = GL_FRONT;
        break;
    case bp3d::driver::ECullingMode::DISABLED:
        pipeline->CullingMode = GL_CULL_FACE;
        break;
    }
    switch (descriptor.RenderMode)
    {
    case bp3d::driver::ERenderMode::TRIANGLES:
        pipeline->RenderMode = GL_FILL;
        break;
    case bp3d::driver::ERenderMode::WIREFRAME:
        pipeline->RenderMode = GL_LINE;
        break;
    }
    pipeline->ScissorEnable = descriptor.ScissorEnable;
    return (pipeline);
}

void GL40ResourceAllocator::FreeFixedConstantBuffer(bp3d::driver::Resource resource)
{
    FreeConstantBuffer(resource);
}

void GL40ResourceAllocator::FreePipeline(bp3d::driver::Resource resource)
{
    Pipeline *pipeline = reinterpret_cast<Pipeline *>(resource);
    bpf::memory::Memory::Free(pipeline);
}

void GL40ResourceAllocator::FreeShaderProgram(bp3d::driver::Resource resource)
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    glDeleteProgram(res.Ptrs[0]);
#else
    glDeleteProgram(res.Ptr);
#endif
}

void GL40ResourceAllocator::FreeBlendState(bp3d::driver::Resource resource)
{
    BlendState *state = reinterpret_cast<BlendState *>(resource);
    bpf::memory::Memory::Free(state);
}

void GL40ResourceAllocator::FreeDepthBuffer(bp3d::driver::Resource resource)
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint rbo = res.Ptrs[0];
#else
    GLuint rbo = res.Ptr;
#endif
    glDeleteRenderbuffers(1, &rbo);
}

void GL40ResourceAllocator::FreeRenderTargetComponent(bp3d::driver::Resource resource)
{
    Texture2D *res = reinterpret_cast<Texture2D *>(resource);
    glDeleteTextures(1, &res->TexId);
    bpf::memory::Memory::Free(res);
}

void GL40ResourceAllocator::FreeRenderTarget(bp3d::driver::Resource resource)
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint fbo = res.Ptrs[0];
#else
    GLuint fbo = res.Ptr;
#endif
    glDeleteFramebuffers(1, &fbo);
}

void GL40ResourceAllocator::FreeTexture2D(bp3d::driver::Resource resource)
{
    Texture2D *res = reinterpret_cast<Texture2D *>(resource);
    glDeleteTextures(1, &res->TexId);
    bpf::memory::Memory::Free(res);
}

void GL40ResourceAllocator::FreeTexture2DArray(bp3d::driver::Resource resource)
{
    Texture2D *res = reinterpret_cast<Texture2D *>(resource);
    glDeleteTextures(1, &res->TexId);
    bpf::memory::Memory::Free(res);
}

void GL40ResourceAllocator::FreeTextureCube(bp3d::driver::Resource resource)
{
    Texture2D *res = reinterpret_cast<Texture2D *>(resource);
    glDeleteTextures(1, &res->TexId);
    bpf::memory::Memory::Free(res);
}

void GL40ResourceAllocator::FreeSampler(bp3d::driver::Resource resource)
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint sampler = res.Ptrs[0];
#else
    GLuint sampler = res.Ptr;
#endif
    glDeleteSamplers(1, &sampler);
}

void GL40ResourceAllocator::FreeConstantBuffer(bp3d::driver::Resource resource)
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint ubo = res.Ptrs[0];
#else
    GLuint ubo = res.Ptr;
#endif
    glDeleteBuffers(1, &ubo);
}

void GL40ResourceAllocator::FreeVertexBuffer(bp3d::driver::Resource resource)
{
#ifdef X86_64
    VertexBuffer res;
    res.Ptr = resource;
    glDeleteVertexArrays(1, &res.Data.VAO);
    glDeleteBuffers(1, &res.Data.VBO);
#else
    VertexBuffer *buf = reinterpret_cast<VertexBuffer *>(resource);
    glDeleteVertexArrays(1, &buf->VAO);
    glDeleteBuffers(1, &buf->VBO);
    bpf::memory::Memory::Free(buf);
#endif
}

void GL40ResourceAllocator::FreeIndexBuffer(bp3d::driver::Resource resource)
{
    ObjectResource res;
    res.Data = resource;
#ifdef X86_64
    GLuint ibo = res.Ptrs[0];
#else
    GLuint ibo = res.Ptr;
#endif
    glDeleteBuffers(1, &ibo);
}

void GL40ResourceAllocator::FreeVertexFormat(bp3d::driver::Resource resource)
{
    VertexFormat *vformat = reinterpret_cast<VertexFormat *>(resource);
    bpf::memory::MemUtils::Delete(vformat);
}
