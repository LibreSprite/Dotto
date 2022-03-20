// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef NO_SDL2

#include <SDL/SDL.h>

#include <common/ColorProfile.hpp>
#include <common/Config.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Window.hpp>
#include <log/Log.hpp>
#include <sdl1/SDLGraphics.hpp>

class SDL1Window : public ui::Window {
public:
    bool wasInit = false;

    void doInit() {
        if (wasInit)
            return;
        wasInit = true;
        *scale = 1;

        globalRect.width = width->toPixel(0, 0);
        globalRect.height = height->toPixel(0, 0);
        localRect.width = globalRect.width;
        localRect.height = globalRect.height;

        id = 0;
        resize();
        setDirty();
    }

    bool update() override {
        doInit();

        if (needResize) {
            int width = getParent()->width->toPixel(0, 0),
                height = getParent()->height->toPixel(0, 0);
            globalRect.width = width / scale;
            globalRect.height = height / scale;
            doResize();
        }
        ui::Window::update();
        setDirty();
        return true;
    }

    void draw(S32 z, Graphics& graphics) override {
        graphics.alpha = 1.0f;
        graphics.scale = scale;
        static_cast<SDLGraphics*>(&graphics)->begin(globalRect, *background);
        ui::Window::draw(z, graphics);
    }
};

static ui::Node::Shared<SDL1Window> win{"sdl1Window"};

#endif
