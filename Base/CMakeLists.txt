cmake_minimum_required(VERSION 3.10)
project(BP3D)

include("${BP_FRAMEWORK_ROOT}/CMakes/Framework.cmake")

set(SOURCES
	include/Engine/Input/EKey.hpp
	include/Engine/Render/IDisplay.hpp
	include/Engine/Render/IRenderEngine.hpp
	include/Engine/Render/IRenderContext.hpp
	include/Engine/Render/IResourceAllocator.hpp
	include/Engine/Render/Event.hpp
	include/Engine/AssetManager.hpp
	src/Engine/AssetManager.cpp
)

bp_setup_module(BP3D BP3D_API)
