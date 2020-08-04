cmake_minimum_required(VERSION 3.10)
project(BP3D.RenderEngine.GL40)

find_package(BPF COMPONENTS Plugin Conan)

conan_add_remote(NAME bincrafters INDEX 1 URL https://api.bintray.com/conan/bincrafters/public-conan)
conan_cmake_run(
    REQUIRES sdl2/2.0.12@bincrafters/stable
    OPTIONS sdl2:sdl2main=False sdl2:shared=True
    BASIC_SETUP CMAKE_TARGETS KEEP_RPATHS
    BUILD missing
)

set(SOURCES
    include/GL40ResourceAllocator.hpp
    include/glad.h
    include/KHR/khrplatform.h
    include/GL40Resources.hpp
    include/GL40RenderContext.hpp
    include/GL40ShaderCompiler.hpp
    include/ShaderDecoder.hpp
    src/glad.c
    src/GL40ResourceAllocator.cpp
    src/GL40RenderContext.cpp
    src/GL40ShaderCompiler.cpp
    src/ShaderDecoder.cpp
)

#This macro will never be used as driver modules do not have any API they map to existing APIs
bp_setup_plugin(${PROJECT_NAME})
bp_use_module(${PROJECT_NAME} BP3D)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::sdl2)

conan_cmake_copy_third_party(SDL2)
