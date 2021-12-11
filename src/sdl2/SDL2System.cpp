// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <SDL2/SDL.h>

#include <common/System.hpp>
#include <log/Log.hpp>

class SDL2System : public System {
public:
    bool running = false;

    bool boot() override {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            Log::write(Log::Level::ERROR, SDL_GetError());
            return false;
        }
        return true;
    }

    std::shared_ptr<Window> openWindow(const PropertySet& properties) override {
        inject<Window> window{"sdl2Window"};
        window->init(properties);
        return window;
    }

    bool run() override {
        pumpEvents();
        return running;
    }

    void pumpEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            }
        }
    }

    ~SDL2System() {
        SDL_Quit();
    }
};

System::Shared<SDL2System> sys{"sdl2"};
