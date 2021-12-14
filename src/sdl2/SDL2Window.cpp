// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <GL/gl.h>
#include <SDL2/SDL.h>

#include <gui/Window.hpp>
#include <log/Log.hpp>

class SDL2Window : public ui::Window {
public:
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;

    bool init(const PropertySet& properties) override {
        Window::init(properties);

        Log::write(Log::Level::VERBOSE,
                   "SDL2Window maximized=", *maximized,
                   " title=", *title,
                   " width=", *width,
                   " height=", *height);

        globalRect.width = width->toPixel(0);
        globalRect.height = height->toPixel(0);
        localRect.width = globalRect.width;
        localRect.height = globalRect.height;

        window = SDL_CreateWindow(title->c_str(), *x, *y, globalRect.width, globalRect.height, SDL_WINDOW_OPENGL);
        if (!window)
            return false;

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        context = SDL_GL_CreateContext(window);
        if (!context)
            return false;

        setDirty();
        return true;
    }

    bool update() override {
        if (!window || !context)
            return false;
        ui::Window::update();
        setDirty();

        glViewport(0, 0, globalRect.width, globalRect.height);
        glClearColor(background->r/255.0f,
                     background->g/255.0f,
                     background->b/255.0f,
                     background->a/255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
        return true;
    }

    ~SDL2Window() {
        if (context)
            SDL_GL_DeleteContext(context);
        if (window)
            SDL_DestroyWindow(window);
    }
};

static ui::Node::Shared<SDL2Window> win{"sdl2Window"};
