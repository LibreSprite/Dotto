// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#if defined(USE_PROFILER)

#include <chrono>

class Profiler {
public:
    using clock = std::chrono::high_resolution_clock;
    using timestamp = clock::time_point;
    using duration = clock::duration;

    Profiler(const char* func);
    ~Profiler();
    static void start();
    static void end();
    static void info(std::string_view str);
private:
    Profiler* parent;
    const char* func;
    timestamp startTime;
    duration childTime;
};

#define PROFILER Profiler profiler_ ## __LINE__ ( __PRETTY_FUNCTION__ );
#define PROFILER_START Profiler::start();
#define PROFILER_END Profiler::end();
#define PROFILER_CALL(cmd) {Profiler pccall_ (#cmd); cmd;}
#define PROFILER_INFO(str) Profiler::info(str);
#else

#define PROFILER
#define PROFILER_START
#define PROFILER_END
#define PROFILER_CALL(cmd) {cmd;}
#define PROFILER_INFO(str) ;
#endif
