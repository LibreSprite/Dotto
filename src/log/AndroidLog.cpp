// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(__ANDROID__)

#include <android/log.h>
#include "Log.hpp"

static const String endline = "\n";

class AndroidLog : public Log {
public:
    String line;

    void internalWrite(const char* string) override {
        if (endline == string) {
            auto type = ANDROID_LOG_WARN;
            if (level == Level::Verbose) type = ANDROID_LOG_VERBOSE;
            else if (level == Level::Info) type = ANDROID_LOG_INFO;
            __android_log_write(type, "DOTTO", line.c_str());
            line.clear();
        } else {
            line += string;
        }
    }
};

static Log::Shared<AndroidLog> logger{"logcat"};

#endif
