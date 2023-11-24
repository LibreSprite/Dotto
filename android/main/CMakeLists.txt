cmake_minimum_required(VERSION 3.4.1)
project(Bacon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/include

        # SDL_internal.h is located in src required by SDL_android_main.c bridge
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/src

        ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_image/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_gfx/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_mixer/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_ttf/include
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SDL2/ ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_image/ ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_image)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_gfx/ ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_gfx)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_mixer/ ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_mixer)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_ttf/ ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_ttf)

add_library(
             main-lib

             SHARED

             SDL_android_main.c
             extractAssets.cpp

             ../../src/GLRenderer.cpp
             ../../src/Material.cpp
             ../../src/Mesh.cpp
             ../../src/Model.cpp
             ../../src/ModelAPI.cpp
             ../../src/Node.cpp
             ../../src/String.cpp
             ../../src/Surface.cpp
             ../../src/VM.cpp
             ../../src/main.cpp
             # main.cpp
           )


find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that you want CMake to locate.
              log )

find_library( # Sets the name of the path variable.
              android-lib
              # Specifies the name of the NDK library that you want CMake to locate.
              android )

target_link_libraries( # Specifies the target library.
                        main-lib
                        SDL2
                        SDL2_image
                        SDL2_gfx
                        SDL2_mixer
                        SDL2_ttf

                        EGL
                        GLESv3
                        ${log-lib}
                        ${android-lib}
                      )
