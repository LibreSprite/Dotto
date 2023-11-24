#pragma once


#ifndef LOGGER
#if __has_include(<SDL.h>)
#define LOGGER SDLLog
#else
#define LOGGER ConsoleLog
#endif
#endif

#if LOGGER==ConsoleLog
#include "ConsoleLog.hpp"
#endif
#if LOGGER==SDLLog
#include "SDLLog.hpp"
#endif

inline LOGGER LOG;
