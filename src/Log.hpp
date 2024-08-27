#pragma once


#ifndef LOGGER
#if __has_include(<SDL.h>)
#define LOGGER SDLLog
#else
#define LOGGER ConsoleLog
#endif
#endif

#ifdef DEBUG_GRAPHICS
#define GFXLOG csLog
#else
#define GFXLOG nullLog
#endif

class NullLog {
public:
    template <typename ... Arg>
    void operator () (Arg&& ... arg) {}
};

#if LOGGER==ConsoleLog
#include "ConsoleLog.hpp"
#endif
#if LOGGER==SDLLog
#include "SDLLog.hpp"
#endif

inline NullLog nullLog;
inline LOGGER Log;

class CSLog {
public:
    template <typename ... Arg>
    void operator () (Arg&& ... arg) {
        std::stringstream ss;
        ((ss << std::forward<Arg>(arg) << ", "), ...);
        Log(ss.str());
    }
};
inline CSLog csLog;

#define LOG Log
