cmake_minimum_required(VERSION 3.10)
project(BP3D.RenderEngine.GL40)

include("../CMakes/Engine.cmake")
include("../CMakes/ThirdParty.cmake")

set(SOURCES
    include/GL40ResourceAllocator.hpp
    include/glad.h
    include/KHR/khrplatform.h
    include/GL40Resources.hpp
    include/GL40RenderContext.hpp
    include/ShaderDecoder.hpp
    include/GL40ShaderCompiler.hpp
    src/glad.c
    src/GL40ResourceAllocator.cpp
    src/GL40RenderContext.cpp
    src/ShaderDecoder.cpp
)

#This macro will never be used as driver modules do not have any API they map to existing APIs
bp_setup_driver(${PROJECT_NAME})
#We provide pre-build libraries for SDL2 on Windows
bp_add_thirdparty(${PROJECT_NAME} SDL2)
