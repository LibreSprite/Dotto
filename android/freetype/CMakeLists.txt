# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
project(freetype)

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

add_definitions(-DFT_CONFIG_CONFIG_H=<ftconfig.h> -DFT2_BUILD_LIBRARY -DFT_CONFIG_MODULES_H=<ftmodule.h>)

add_library( # Sets the name of the library.
        freetype

        # Sets the library as a shared library.
        SHARED

        src/sfnt/sfobjs.c
        src/sfnt/sfntpic.c
        src/sfnt/sfdriver.c
        src/sfnt/ttmtx.c
        src/sfnt/ttload.c
        src/sfnt/ttbdf.c
        src/sfnt/ttpost.c
        src/sfnt/ttcmap.c
        src/sfnt/ttkern.c
        src/sfnt/ttsbit.c
        src/base/md5.c
        src/base/ftglyph.c
        src/base/ftotval.c
        src/base/ftinit.c
        src/base/ftbdf.c
        src/base/ftgasp.c
        src/base/ftsystem.c
        src/base/ftstroke.c
        src/base/ftapi.c
        src/base/ftlcdfil.c
        src/base/ftdebug.c
        src/base/ftcid.c
        src/base/ftfstype.c
        src/base/ftxf86.c
        src/base/ftbitmap.c
        src/base/ftpfr.c
        src/base/ftwinfnt.c
        src/base/ftgxval.c
        src/base/ftpatent.c
        src/base/ftbbox.c
        src/base/ftbase.c
        src/base/ftsynth.c
        src/base/fttype1.c
        src/base/ftmm.c
        src/smooth/ftsmooth.c
        src/smooth/ftgrays.c
        src/smooth/ftspic.c
        src/raster/ftraster.c
        src/raster/rastpic.c
        src/raster/ftrend1.c
        src/bzip2/ftbzip2.c
        src/truetype/ttobjs.c
        src/truetype/ttgload.c
        src/truetype/ttpic.c
        src/truetype/ttgxvar.c
        src/truetype/ttsubpix.c
        src/truetype/ttinterp.c
        src/truetype/ttdriver.c
        src/truetype/ttpload.c
        )


target_link_libraries(
        freetype
        GLESv1_CM
        GLESv2
        android
        z
        log
)