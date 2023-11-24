# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(SDL2_image)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

find_library( # Sets the name of the path variable.
        GLESv1_CM

        # Specifies the name of the NDK library that
        # you want CMake to locate.
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
        log
        log
)

include_directories(
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_png/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_jpeg/include
)

add_definitions(-DGL_GLEXT_PROTOTYPES -DLOAD_XPM -DLOAD_PNG -DLOAD_JPG)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_png/ ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_png/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_jpeg/ ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2_jpeg/)

add_library( # Sets the name of the library.
        SDL2_image

        # Sets the library as a shared library.
        SHARED



        src/IMG.c
        src/IMG_gif.c
        src/IMG_png.c
        src/IMG_bmp.c
        src/IMG_xxx.c
        src/IMG_pcx.c
        src/IMG_xv.c
        src/IMG_jpg.c
        src/IMG_xpm.c
        src/IMG_webp.c
        src/IMG_tga.c
        src/IMG_tif.c
        # src/showimage.c
        src/IMG_xcf.c
        src/IMG_pnm.c
        src/IMG_lbm.c


        )


target_link_libraries(
        SDL2_image
        SDL2_png
        SDL2_jpeg
        SDL2
        GLESv1_CM
        GLESv2
        android
        log
        )
