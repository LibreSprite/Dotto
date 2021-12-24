// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>

class Log : public Injectable<Log> {
public:

    enum class Level {
        VERBOSE,
        INFO,
        ERROR
    };

    template<typename ... Args>
    static void write(Level level, Args&& ... args) {
        if (instance && instance->level <= level) {
            (instance->writeArg(std::forward<Args>(args)),...);
            instance->internalWrite("\n");
        }
    }

    void setLevel(Level level) {
        this->level = level;
    }

    void setGlobal() {
        instance = this;
    }

    ~Log() {
        if (instance == this)
            instance = nullptr;
    }

protected:
    Level level;
    virtual void internalWrite(const char* string) = 0;

private:
    static inline Log* instance = nullptr;

    void writeArg(const String& str) {internalWrite(str.c_str());}
    void writeArg(const char* str) {internalWrite(str);}
    void writeArg(char* str) {internalWrite(str);}

    template<typename Arg>
    void writeArg(Arg value) {
        if constexpr (std::is_convertible_v<Arg, String>) {
            internalWrite(static_cast<String>(value).c_str());
        } else {
            internalWrite(std::to_string(value).c_str());
        }
    }
};

template<typename ... Args>
inline bool logV(Args&& ... args) {
    Log::write(Log::Level::VERBOSE, std::forward<Args>(args)...);
    return true;
}

template<typename ... Args>
inline bool logI(Args&& ... args) {
    Log::write(Log::Level::INFO, std::forward<Args>(args)...);
    return true;
}

template<typename ... Args>
inline bool logE(Args&& ... args) {
    Log::write(Log::Level::ERROR, std::forward<Args>(args)...);
    return true;
}
