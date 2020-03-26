cmake_minimum_required(VERSION 3.10)
project(BlockProject)

include(ExternalProject)

if (APPLE)
    set(FRAMEWORK "MacOSX-x64" CACHE STRING "Framework root directory")
elseif (UNIX)
    set(FRAMEWORK "Linux-x64" CACHE STRING "Framework root directory")
else ()
    set(FRAMEWORK "Windows-x64" CACHE STRING "Framework root directory")
endif ()

set(BP_FRAMEWORK_ROOT "${CMAKE_CURRENT_LIST_DIR}/Framework-${FRAMEWORK}")

add_subdirectory(Base)