cmake_minimum_required(VERSION 3.10)
project(BP3D.Tests)

include("${BP_FRAMEWORK_ROOT}/CMakes/Program.cmake")
include("../CMakes/Engine.cmake")

configure_file(CMakeLists.in.txt googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif (result)
execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif (result)

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)
				 
include_directories("${gtest_SOURCE_DIR}/include")

set(SOURCES
    include/ListLogHandler.hpp
    src/ListLogHandler.cpp
    src/main.cpp
    src/AssetManager.cpp
    src/BPX.cpp
)

bp_setup_program(${PROJECT_NAME} include)
bp_add_module(${PROJECT_NAME} BP3D)
target_link_libraries(${PROJECT_NAME} PRIVATE gtest_main)
