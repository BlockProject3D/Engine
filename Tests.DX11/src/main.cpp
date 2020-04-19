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

#include <Engine/Driver/IRenderEngine.hpp>
#include <Framework/IO/ConsoleWriter.hpp>
#include <Framework/IO/FileStream.hpp>
#include <Framework/IO/TextReader.hpp>
#include <Framework/System/IApplication.hpp>
#include <Framework/System/ModuleManager.hpp>
#include <Framework/System/Platform.hpp>

using namespace bpf::system;
using namespace bpf::collection;
using namespace bpf::io;
using namespace bpf;

bp3d::driver::Resource AttemptBuildShaderProg(const Paths &paths, const memory::UniquePtr<bp3d::driver::IRenderEngine> &re, bp3d::driver::IResourceAllocator &allocator)
{
    auto newLine = Platform::GetOSInfo().NewLine;
    String vShader;
    String pShader;
    {
        FileStream stream(paths.AppRoot() + "../../Tests.DX11/test.vertex.hlsl", FILE_MODE_READ);
        TextReader reader(stream);
        vShader = reader.ReadAll();
    }
    {
        FileStream stream(paths.AppRoot() + "../../Tests.DX11/test.pixel.hlsl", FILE_MODE_READ);
        TextReader reader(stream);
        pShader = reader.ReadAll();
    }
    auto compiler = re->CreateShaderCompiler();
    compiler->SetCompileFlags(bp3d::driver::SHADER_COMPILE_DEBUG | bp3d::driver::SHADER_COMPILE_O0);
    auto vShaderObject = compiler->Compile(vShader, "TestVertexShader", bp3d::driver::EShaderType::VERTEX);
    auto pShaderObject = compiler->Compile(pShader, "TestPixelShader", bp3d::driver::EShaderType::PIXEL);
    bpf::io::ConsoleWriter writer;
    for (auto &bind : vShaderObject->GetBindings())
    {
        bpf::String typeN;
        switch (bind.Type)
        {
        case bp3d::driver::EBindingType::CONSTANT_BUFFER:
            typeN = "CONSTANT_BUFFER";
            break;
        case bp3d::driver::EBindingType::SAMPLER:
            typeN = "SAMPLER";
            break;
        case bp3d::driver::EBindingType::TEXTURE:
            typeN = "TEXTURE";
            break;
        }
        writer << "Found binding: Stage=Vertex, Register=" << bind.Register << ", Type=" << typeN << ", Name=" << bind.Name << newLine;
    }
    for (auto &bind : pShaderObject->GetBindings())
    {
        bpf::String typeN;
        switch (bind.Type)
        {
        case bp3d::driver::EBindingType::CONSTANT_BUFFER:
            typeN = "CONSTANT_BUFFER";
            break;
        case bp3d::driver::EBindingType::SAMPLER:
            typeN = "SAMPLER";
            break;
        case bp3d::driver::EBindingType::TEXTURE:
            typeN = "TEXTURE";
            break;
        }
        writer << "Found binding: Stage=Pixel, Register=" << bind.Register << ", Type=" << typeN << ", Name=" << bind.Name << newLine;
    }
    for (auto &out : pShaderObject->GetPixelOutputs())
        writer << "Found pixel stage output: Register=" << out.Register << ", Channels=" << out.Channels << ", Name=" << out.Name << newLine;
    compiler->Link();
    ByteBuf vShaderCode = vShaderObject->ToByteBuf();
    ByteBuf pShaderCode = pShaderObject->ToByteBuf();
    bp3d::driver::ShaderProgramDescriptor prog;
    bp3d::driver::ShaderDescriptor desc;
    desc.Data = *vShaderCode;
    desc.Size = vShaderCode.Size();
    desc.Type = bp3d::driver::EShaderType::VERTEX;
    prog.Shaders.Add(desc);
    desc.Data = *pShaderCode;
    desc.Size = pShaderCode.Size();
    desc.Type = bp3d::driver::EShaderType::PIXEL;
    prog.Shaders.Add(desc);
    bp3d::driver::ShaderBindingDescriptor bind;
    bind.Register = 0;
    bind.StageFlags = bp3d::driver::LOCK_PIXEL_STAGE;
    bind.Type = bp3d::driver::EBindingType::TEXTURE;
    prog.Bindings.Add(bind);
    bind.Type = bp3d::driver::EBindingType::SAMPLER;
    prog.Bindings.Add(bind);
    prog.RenderTargetOutputTypes.Add(bp3d::driver::ETextureFormat::RGBA_UINT_8);
    return (allocator.AllocShaderProgram(prog));
}

bp3d::driver::Resource AttemptBuildTexture(bp3d::driver::IResourceAllocator &allocator)
{
    bpf::uint8 textureData[16] = {
        0, 0, 0, 1, /* */ 255, 0, 255, 1,
        255, 0, 255, 1, /* */ 0, 0, 0, 1};
    bp3d::driver::TextureDescriptor desc;

    desc.Compression = bp3d::driver::ETextureCompression::NONE;
    desc.Data = textureData;
    desc.Format = bp3d::driver::ETextureFormat::RGBA_UINT_8;
    desc.MipMaps = 1;
    desc.QualityLevel = 0;
    desc.SampleLevel = 1;
    desc.Width = 2;
    desc.Height = 2;
    return (allocator.AllocTexture2D(bp3d::driver::EBufferType::STATIC, desc));
}

bp3d::driver::Resource AttemptBuildSampler(bp3d::driver::IResourceAllocator &allocator)
{
    bp3d::driver::SamplerDescriptor desc;

    desc.AddressModeU = bp3d::driver::ETextureAddressing::MIRRORED_REPEAT;
    desc.AddressModeV = bp3d::driver::ETextureAddressing::MIRRORED_REPEAT;
    desc.AddressModeW = bp3d::driver::ETextureAddressing::MIRRORED_REPEAT;
    desc.AnisotropicLevel = 0;
    desc.FilterFunc = bp3d::driver::ETextureFiltering::MIN_MAG_POINT_MIPMAP_POINT;
    return (allocator.AllocSampler(desc));
}

bp3d::driver::Resource AttemptBuildPipeline(bp3d::driver::IResourceAllocator &allocator, bp3d::driver::Resource shaderProg, bp3d::driver::Resource vformat)
{
    bp3d::driver::PipelineDescriptor desc;
    desc.BlendState = Null;
    desc.DepthEnable = false;
    desc.DepthWriteEnable = false;
    desc.CullingMode = bp3d::driver::ECullingMode::BACK_FACE;
    desc.RenderMode = bp3d::driver::ERenderMode::TRIANGLES;
    desc.ScissorEnable = false;
    desc.ShaderProgram = shaderProg;
    desc.VertexFormat = vformat;
    return (allocator.AllocPipeline(desc));
}

int Main(IApplication &app, const Array<String> &args, const Paths &paths)
{
    ConsoleWriter writer;
    auto newLine = Platform::GetOSInfo().NewLine;
    ModuleManager<bp3d::driver::IRenderEngine> mdManager(paths.AppRoot());

    writer.WriteLine("Welcome to the DX11 testing program");
    writer << "App root: " << paths.AppRoot().Path() << newLine;
    writer.WriteLine("Loading RenderEngine.DX11 module...");
    try
    {
        mdManager.LoadModule("RenderEngine", "BP3D.RenderEngine.DX11");
        auto &ptr = mdManager.GetModule(Name("RenderEngine"));
        auto &props = ptr->GetProperties();
        writer << "DriverName = " << props.DriverName << newLine;
        writer << "FlipTextureSpace = " << props.FlipTextureSpace << newLine;
        writer << "SupportsRGB = " << props.SupportsRGB << newLine;
        writer << "SeparateVertexFormat = " << props.SeparateVertexFormat << newLine;
        writer << "SupportsMultiBlending = " << props.SupportsMultiBlending << newLine;
        writer.WriteLine("Listing display modes...");
        writer.Flush();
        auto modes = ptr->GetDisplayModes();
        bp3d::driver::DisplayMode wanted;
        for (auto &mode : modes)
        {
            writer << "\t{ Id=" << mode.Id << ", Width=" << mode.Width << ", Height=" << mode.Height << ", Fullscreen=" << mode.Fullscreen << ", IsVR=" << mode.IsVR << " }" << newLine;
            if (mode.Width == 1920U && mode.Height == 1080U && mode.Fullscreen == false)
                wanted = mode;
        }
        writer.Flush();
        bp3d::driver::RenderProperties rprops;
        rprops.VSync = true;
        rprops.Antialiasing = true;
        rprops.Multisampling = true;
        auto display = ptr->CreateStandardDisplay(app, "Testing DirectX 11", wanted, rprops);
        auto p = display->GetContextProperties();
        writer.WriteLine("Context properties:");
        writer << "HardwareName = " << p.HardwareName << newLine;
        writer << "MaxTextureWidth = " << p.MaxTextureWidth << newLine;
        writer << "MaxTextureHeight = " << p.MaxTextureHeight << newLine;
        writer << "MaxAnisotropyLevel = " << p.MaxAnisotropicLevel << newLine;
        writer << "MaxVRAM = " << p.MaxVRAM << newLine;
        writer << "MaxImageQuality = " << p.MaxImageQuality << newLine;
        writer.Flush();
        bool shouldClose = false;
        bp3d::driver::VertexFormatDescriptor desc;
        bp3d::driver::VertexComponent comp;
        comp.Name = "Position";
        comp.Type = bp3d::driver::EVertexComponentType::VECTOR_FLOAT_2;
        desc.Components.Add(comp);
        comp.Name = "TexCoords";
        comp.Type = bp3d::driver::EVertexComponentType::VECTOR_FLOAT_2;
        desc.Components.Add(comp);
        desc.Name = "Vertex2D";
        auto vertex2d = display->GetContext().GetResourceAllocator().AllocVertexFormat(desc);
        float verts[] = {
            -0.5f, 0.5f, 0, 0, //Format vX, vY, vU, vV
            0.5f, -0.5f, 1, 1,
            0.5f, 0.5f, 1, 0,
            -0.5f, 0.5f, 0, 0,
            -0.5f, -0.5f, 0, 1,
            0.5f, -0.5f, 1, 1};
        bp3d::driver::BufferDescriptor bdesc;
        bdesc.Data = verts;
        bdesc.Size = sizeof(float) * 4 * 6;
        auto vbuf = display->GetContext().GetResourceAllocator().AllocVertexBuffer(bp3d::driver::EBufferType::STATIC, vertex2d, bdesc);
        auto shader = AttemptBuildShaderProg(paths, ptr, display->GetContext().GetResourceAllocator());
        auto pipeline = AttemptBuildPipeline(display->GetContext().GetResourceAllocator(), shader, vertex2d);
        auto texture = AttemptBuildTexture(display->GetContext().GetResourceAllocator());
        auto sampler = AttemptBuildSampler(display->GetContext().GetResourceAllocator());
        display->GetContext().SetViewport(0, 0, 1920U, 1080U);
        display->GetContext().LockVertexBuffer(vbuf, 4 * sizeof(float));
        while (!shouldClose)
        {
            bp3d::driver::Event ev;
            if (display->PollEvent(ev))
                shouldClose = true;
            //Render code here
            display->GetContext().Clear(true, true);
            display->GetContext().LockPipeline(pipeline);
            display->GetContext().LockTexture(texture, 0);
            display->GetContext().LockSampler(sampler, 0);
            display->GetContext().SetRenderTarget(Null);
            display->GetContext().Draw(0, 6);
            //End
            display->Update();
        }
        display->GetContext().GetResourceAllocator().FreeShaderProgram(shader);
        display->GetContext().GetResourceAllocator().FreePipeline(pipeline);
        display->GetContext().GetResourceAllocator().FreeSampler(sampler);
        display->GetContext().GetResourceAllocator().FreeTexture2D(texture);
        display->GetContext().GetResourceAllocator().FreeVertexBuffer(vbuf);
        display->GetContext().GetResourceAllocator().FreeVertexFormat(vertex2d);
    }
    catch (const ModuleException &ex)
    {
        writer << Console::TextStyle(EConsoleColor::RED) << "An error has occured while loading module: " << ex.Message() << newLine << Console::ClearTextStyle();
    }
    catch (const RuntimeException &ex)
    {
        writer << Console::TextStyle(EConsoleColor::RED) << "An error has occured while loading rendering engine: " << ex.Type() << "> " << ex.Message() << newLine << Console::ClearTextStyle();
    }
    return (0);
}
