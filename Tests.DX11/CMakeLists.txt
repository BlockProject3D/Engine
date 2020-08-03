cmake_minimum_required(VERSION 3.10)
project(BP3D.Tests.DX11)

include("${CMAKE_CURRENT_LIST_DIR}/../CMakes/Program.cmake")

set(SOURCES
    src/LowLevelMain.cpp
    src/main.cpp
)

bp_setup_program(${PROJECT_NAME} include)
bp_use_module(${PROJECT_NAME} BP3D)
