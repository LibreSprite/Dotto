
cmake_minimum_required(VERSION 3.4.1)
project(SDL2_mixer)


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
        SDL2_mixer

        # Sets the library as a shared library.
        SHARED

        src/music_mad.c
        src/dynamic_flac.c
        src/load_aiff.c
        src/load_voc.c
        src/dynamic_ogg.c
        src/load_mp3.c
        src/mixer.c
        # src/playmus.c
        src/music_ogg.c
        src/music_cmd.c
        src/music_mod.c
        src/load_flac.c
        src/dynamic_fluidsynth.c
        src/fluidsynth.c
        src/effect_stereoreverse.c
        src/music.c
        src/dynamic_mod.c
        src/effects_internal.c
        src/dynamic_modplug.c
        src/dynamic_mp3.c
        src/load_ogg.c
        src/wavestream.c
        src/effect_position.c
        src/music_modplug.c
        src/music_flac.c
        )


target_link_libraries(
        SDL2_mixer
        SDL2
        GLESv1_CM
        GLESv2
        android
        z
        log
)
