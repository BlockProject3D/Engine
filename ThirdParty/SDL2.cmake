if (WIN32)
    set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/SDL2/include)
    set(LIB ${CMAKE_CURRENT_LIST_DIR}/SDL2/SDL2.lib)
else (WIN32)
    find_file(INCLUDE_DIR NAME SDL.h HINTS SDL2)
    find_library(LIB NAME SDL2)
endif (WIN32)
