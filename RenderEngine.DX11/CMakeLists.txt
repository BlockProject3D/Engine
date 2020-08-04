cmake_minimum_required(VERSION 3.10)
project(BP3D.RenderEngine.DX11)

find_package(BPF COMPONENTS Plugin)
include("../CMakes/ThirdParty.cmake")

set(SOURCES
    include/DX11ResourceAllocator.hpp
    include/DX11Resources.hpp
    include/DX11RenderContext.hpp
    include/DX11Display.hpp
    include/DX11RenderEngine.hpp
    include/DX11ShaderCompiler.hpp
    src/DX11ResourceAllocator.cpp
    src/DX11RenderContext.cpp
    src/DX11Display.cpp
    src/DX11RenderEngine.cpp
    src/DX11ShaderCompiler.cpp
)

#This macro will never be used as driver modules do not have any API they map to existing APIs
bp_setup_plugin(${PROJECT_NAME})
bp_use_module(${PROJECT_NAME} BP3D)
target_link_libraries(${PROJECT_NAME} PRIVATE d3dcompiler.lib d3d11.lib dxgi.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE debug DXGuid.lib)
bp_add_thirdparty(${PROJECT_NAME} SDL2)
