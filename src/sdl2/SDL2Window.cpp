// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <SDL2/SDL.h>

#include <common/Window.hpp>
#include <log/Log.hpp>

class SDL2Window : public Window {
public:
    bool init(const PropertySet& properties) override {
        Window::init(properties);
        Log::write(Log::Level::VERBOSE,
                   "SDL2Window maximized=", *maximized,
                   " title=", *title,
                   " width=", *width,
                   " height=", *height);
        return true;
    }
};

static Window::Shared<SDL2Window> win{"sdl2Window"};
