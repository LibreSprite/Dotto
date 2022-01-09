// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <SDL2/SDL.h>

#include <gui/GLGraphics.hpp>
#include <gui/Window.hpp>
#include <log/Log.hpp>

class SDL2Window : public ui::Window {
public:
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    bool wasInit = false;
    std::shared_ptr<GLGraphics> graphics = std::make_shared<GLGraphics>();

    void doInit() {
        if (wasInit)
            return;
        wasInit = true;

        globalRect.width = width->toPixel(0, 0);
        globalRect.height = height->toPixel(0, 0);
        localRect.width = globalRect.width;
        localRect.height = globalRect.height;

        window = SDL_CreateWindow(title->c_str(), *x, *y, globalRect.width, globalRect.height, SDL_WINDOW_OPENGL/* | SDL_WINDOW_RESIZABLE*/);
        if (!window)
            return;

        id = SDL_GetWindowID(window);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        context = SDL_GL_CreateContext(window);
        if (!context)
            return;

        SDL_GL_MakeCurrent(window, context);
        graphics->init();

        setDirty();
    }

    bool update() override {
        doInit();
        if (!window || !context)
            return false;
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        if (width / scale != globalRect.width ||
            height / scale != globalRect.height ||
            needResize) {
            globalRect.width = width / scale;
            globalRect.height = height / scale;
            doResize();
        }
        ui::Window::update();
        setDirty();
        return true;
    }

    void draw(S32 z, Graphics&) override {
        SDL_GL_MakeCurrent(window, context);
        graphics->scale = scale;
        graphics->begin(globalRect, *background);
        ui::Window::draw(z, *graphics.get());
        graphics->end();
        SDL_GL_SwapWindow(window);
    }

    ~SDL2Window() {
        if (context && window) {
            SDL_GL_MakeCurrent(window, context);
            graphics.reset();
        }

        if (context)
            SDL_GL_DeleteContext(context);
        if (window)
            SDL_DestroyWindow(window);
    }
};

static ui::Node::Shared<SDL2Window> win{"sdl2Window"};
