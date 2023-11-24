# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(SDL2_jpeg)

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
        SDL2_jpeg

        # Sets the library as a shared library.
        SHARED

        src/rdswitch.c
        src/jquant1.c
        src/rdbmp.c
        src/jcmainct.c
        src/jcdctmgr.c
        src/jcsample.c
        src/wrgif.c
        src/jdmaster.c
        src/jdatadst.c
        src/jaricom.c
        src/jutils.c
        src/rdgif.c
        src/jdcolor.c
        src/rdppm.c
        src/jchuff.c
        src/wrppm.c
        src/jquant2.c
        src/jcmarker.c
        src/rdtarga.c
        src/rdrle.c
        src/jdtrans.c
        src/jcmaster.c
        src/jfdctfst.c
        src/jdapimin.c
        src/jccoefct.c
        src/jidctfst.c
        src/wrbmp.c
        src/jddctmgr.c
        src/jdmerge.c
        src/jdarith.c
        src/jidctint.c
        src/jdatasrc.c
        src/jcomapi.c
        src/wrrle.c
        src/rdcolmap.c
        src/jmemmgr.c
        src/jcinit.c
        src/wrtarga.c
        src/jidctflt.c
        src/jmem-android.c
        src/jdapistd.c
        src/jerror.c
        src/jcparam.c
        src/jcprepct.c
        src/jdsample.c
        src/jdhuff.c
        src/cdjpeg.c
        src/jdpostct.c
        src/jdmarker.c
        src/jfdctint.c
        src/jcarith.c
        src/jdcoefct.c
        src/jcapimin.c
        src/transupp.c
        src/jfdctflt.c
        src/jccolor.c
        src/jdmainct.c
        src/jdinput.c
        src/wrjpgcom.c
        src/jctrans.c
        src/jcapistd.c
        )


target_link_libraries(
        SDL2_jpeg
        SDL2
        GLESv1_CM
        GLESv2
        android
        z
        log
)