#pragma once

#if __has_include(<SDL.h>)

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_syswm.h>
#include <SDL_video.h>

#include "Events.hpp"
#include "Model.hpp"
#include "GLRenderer.hpp"

template<typename Log>
class SDL2Graphics {
public:
    Log log;
    bool running;
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    GLRenderer renderer;

    SDL2Graphics() {
        running = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS) == 0;
    }

    ~SDL2Graphics() {
        if (context && window) {
            SDL_GL_MakeCurrent(window, context);
            renderer.shutdown();
        }
        if (context)
            SDL_GL_DeleteContext(context);
        if (window)
            SDL_DestroyWindow(window);
        SDL_Quit();
    }

    int width() {
        if (!window)
            return 100;
        int w, h;
	#if SDL_VERSION_ATLEAST(2, 26, 0)
        SDL_GetWindowSizeInPixels(window, &w, &h);
	#else
        SDL_GetWindowSize(window, &w, &h);
	#endif
        return w;
    }

    int height() {
        if (!window)
            return 100;
        int w, h;
	#if SDL_VERSION_ATLEAST(2, 26, 0)
        SDL_GetWindowSizeInPixels(window, &w, &h);
	#else
        SDL_GetWindowSize(window, &w, &h);
	#endif
        return h;
    }

    ON(Boot) {
        auto cfg = Model::main->get("window", std::make_shared<Model>());
        auto title = cfg->get("title", "MicroDIRT");
        int width = cfg->get("width", 320.0f);
        int height = cfg->get("height", 200.0f);
        int x = cfg->get("x", float(SDL_WINDOWPOS_CENTERED));
        int y = cfg->get("y", float(SDL_WINDOWPOS_CENTERED));

        window = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (!window) {
            running = false;
            log("Could not create window");
            return;
        }

        // id = SDL_GetWindowID(window);

        int oglMajor = 3;
        int oglMinor = 0;
	std::string oglProfile = "es";

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        context = SDL_GL_CreateContext(window);
#else
	oglMajor = cfg->get("OpenGLMajor", 0.0f);
	oglMinor = cfg->get("OpenGLMinor", 0.0f);
        oglProfile = cfg->get("OpenGLProfile", "es");
        if (oglMajor == 0) {
            oglMajor = 3;
            oglMinor = 3;
            oglProfile = "core";
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, oglMajor);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, oglMinor);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, oglProfile == "es" ? SDL_GL_CONTEXT_PROFILE_ES : SDL_GL_CONTEXT_PROFILE_CORE);
        context = SDL_GL_CreateContext(window);

        if (!context) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            context = SDL_GL_CreateContext(window);
	    oglMajor = 3;
	    oglMinor = 1;
	    oglProfile = "es";
        }
#endif
        if (!context) {
            log("Could not create OpenGL context");
            running = false;
            return;
        }

        auto version = std::to_string(oglMajor) + std::to_string(oglMinor) + "0 " + oglProfile;
        log("OpenGL ", version);
        renderer.init(oglMajor, oglMinor, oglProfile);
    }

    ON(Draw) {
        if (!running || !window || !context)
            return;
        SDL_GL_MakeCurrent(window, context);
	if (Scene::main) {
	    renderer.draw(Scene::main);
	}
        SDL_GL_SwapWindow(window);
    }

    ON(PreUpdate) {
        if (!window || !running)
            return;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
		Model::main->set("mouseX", float(event.button.x));
		Model::main->set("mouseY", float(event.button.y));
                if (event.button.button == 1)
                    emit(EventId::MouseLeftDown);
                if (event.button.button == 2)
                    emit(EventId::MouseMiddleDown);
                if (event.button.button == 3)
                    emit(EventId::MouseRightDown);
                break;

            case SDL_MOUSEBUTTONUP:
		Model::main->set("mouseX", float(event.button.x));
		Model::main->set("mouseY", float(event.button.y));
                if (event.button.button == 1)
                    emit(EventId::MouseLeftUp);
                if (event.button.button == 2)
                    emit(EventId::MouseMiddleUp);
                if (event.button.button == 3)
                    emit(EventId::MouseRightUp);
                break;

            case SDL_MOUSEMOTION:
		Model::main->set("mouseX", float(event.button.x));
		Model::main->set("mouseY", float(event.button.y));
                emit(EventId::MouseMove);
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event)  {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    emit(EventId::Resize);
                    break;
                }
            }
        }
    }
};

template<typename Log>
using Graphics = SDL2Graphics<Log>;

#endif
