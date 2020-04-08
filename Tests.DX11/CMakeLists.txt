cmake_minimum_required(VERSION 3.10)
project(BP3D.Tests.DX11)

include("${BP_FRAMEWORK_ROOT}/CMakes/Program.cmake")
include("../CMakes/Engine.cmake")

set(SOURCES
    src/LowLevelMain.cpp
    src/main.cpp
)

bp_setup_program(${PROJECT_NAME} include)
bp_add_module(${PROJECT_NAME} BP3D)
