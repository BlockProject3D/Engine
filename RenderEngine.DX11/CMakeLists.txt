cmake_minimum_required(VERSION 3.10)
project(BP3D.RenderEngine.DX11)

find_package(BPF COMPONENTS Plugin Conan)

conan_add_remote(NAME bp3d INDEX 1 URL https://api.bintray.com/conan/yuri6037/bp3d)
conan_add_remote(NAME bincrafters INDEX 2 URL https://api.bintray.com/conan/bincrafters/public-conan)
conan_cmake_run(
    REQUIRES sdl2/2.0.12@bp3d/stable
    BASIC_SETUP CMAKE_TARGETS KEEP_RPATHS
    BUILD missing
)

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
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::sdl2)
target_link_libraries(${PROJECT_NAME} PRIVATE d3dcompiler.lib d3d11.lib dxgi.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE debug DXGuid.lib)

conan_cmake_copy_third_party(SDL2)
