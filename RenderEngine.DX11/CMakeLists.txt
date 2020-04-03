cmake_minimum_required(VERSION 3.10)
project(BP3D.RenderEngine.DX11)

include("../CMakes/Engine.cmake")

set(SOURCES
	include/DX11ResourceAllocator.hpp
	include/DX11Resources.hpp
	src/DX11ResourceAllocator.cpp
)

#This macro will never be used as driver modules do not have any API they map to existing APIs
bp_setup_driver(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE d3dcompiler.lib d3d11.lib)
