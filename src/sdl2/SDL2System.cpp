// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <SDL2/SDL.h>
#include <common/System.hpp>
#include <log/Log.hpp>

class SDL2System : public System {
public:
    bool boot() override {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            Log::write(Log::Level::ERROR, SDL_GetError());
            return false;
        }
        return true;
    }

    ~SDL2System() {
        SDL_Quit();
    }
};

System::Shared<SDL2System> sys{"sdl2"};
