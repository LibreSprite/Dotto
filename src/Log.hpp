#pragma once

#ifndef LOGGER
#define LOGGER ConsoleLog
#endif

#if LOGGER==ConsoleLog
#include "ConsoleLog.hpp"
#endif

inline LOGGER LOG;
