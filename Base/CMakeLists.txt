cmake_minimum_required(VERSION 3.10)
project(BP3D)

include("${BP_FRAMEWORK_ROOT}/CMakes/Framework.cmake")

set(SOURCES
    include/Engine/Driver/BufferDescriptor.hpp
    include/Engine/Driver/PipelineDescriptor.hpp
    include/Engine/Driver/Event.hpp
    include/Engine/Driver/IDisplay.hpp
    include/Engine/Driver/IRenderContext.hpp
    include/Engine/Driver/IRenderEngine.hpp
    include/Engine/Driver/IResourceAllocator.hpp
    include/Engine/Driver/RenderTargetDescriptor.hpp
    include/Engine/Driver/Resource.hpp
    include/Engine/Driver/SamplerDescriptor.hpp
    include/Engine/Driver/ShaderProgramDescriptor.hpp
    include/Engine/Driver/TextureDescriptor.hpp
    include/Engine/Driver/VertexFormatDescriptor.hpp
    include/Engine/Driver/BlendStateDescriptor.hpp
    include/Engine/Driver/IShaderCompiler.hpp
    include/Engine/Driver/IShader.hpp
    include/Engine/Input/EKey.hpp
    include/Engine/Asset/ConstantBuffer.hpp
    include/Engine/Asset/Model.hpp
    include/Engine/Asset/Sampler.hpp
    include/Engine/Asset/Shader.hpp
    include/Engine/Asset/Texture.hpp
    include/Engine/IAssetProvider.hpp
    include/Engine/IAssetBuilder.hpp
    include/Engine/AssetBuildThread.hpp
    include/Engine/AssetManager.hpp
    include/Engine/Asset.hpp
    src/Engine/AssetManager.cpp
    src/Engine/AssetBuildThread.cpp
)

bp_setup_module(BP3D BP3D_API)
