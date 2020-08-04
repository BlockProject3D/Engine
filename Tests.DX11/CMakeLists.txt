cmake_minimum_required(VERSION 3.10)
project(BP3D.Tests.DX11)

find_package(BPF COMPONENTS Program)

set(SOURCES
    src/LowLevelMain.cpp
    src/main.cpp
)

bp_setup_program(${PROJECT_NAME})
bp_use_module(${PROJECT_NAME} BP3D)
