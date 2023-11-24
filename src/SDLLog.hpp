#pragma once

#include <sstream>
#include <SDL.h>

class SDLLog {
public:
    template <typename ... Arg>
    void operator () (Arg&& ... arg) {
        std::stringstream ss;
        ((ss << std::forward<Arg>(arg)), ...);
        ss << std::endl;
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", ss.str().c_str());
    }
};
