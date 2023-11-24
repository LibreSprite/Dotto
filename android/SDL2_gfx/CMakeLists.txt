
cmake_minimum_required(VERSION 3.4.1)
project(SDL2_gfx)


find_library( # Sets the name of the path variable.
        GLESv1_CM

        GLESv1_CM )

find_library(
        GLESv2
        GLESv2
)

find_library(
        android
        android
)

find_library(
        z
        z
)

find_library(
        log
        log
)

include_directories(include)

add_definitions(-DGL_GLEXT_PROTOTYPES)

add_library( # Sets the name of the library.
        SDL2_gfx

        # Sets the library as a shared library.
        SHARED

        src/SDL2_framerate.c
        src/SDL2_gfxPrimitives.c
        src/SDL2_rotozoom.c
        src/SDL2_imageFilter.c
        )


target_link_libraries(
        SDL2_gfx
        SDL2
        GLESv1_CM
        GLESv2
        android
        z
        log
)