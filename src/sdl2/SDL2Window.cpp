// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef USE_SDL2

#include <SDL.h>

#include <common/ColorProfile.hpp>
#include <common/Config.hpp>
#include <fs/FileSystem.hpp>
#include <gui/GLGraphics.hpp>
#include <gui/Window.hpp>
#include <log/Log.hpp>
#include <sys/NativeWindowPlugin.hpp>

#include <SDL_syswm.h>

class SDL2Window : public ui::Window {
public:
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;

    bool wasInit = false;
    std::shared_ptr<GLGraphics> graphics = std::make_shared<GLGraphics>();
    std::shared_ptr<ColorProfile> profile;

    std::vector<std::pair<String, inject<NativeWindowPlugin>>> plugins;

    void doInit() {
        if (wasInit)
            return;
        wasInit = true;

        globalRect.width = width->toPixel(0, 0);
        globalRect.height = height->toPixel(0, 0);
        localRect.width = globalRect.width;
        localRect.height = globalRect.height;

        window = SDL_CreateWindow(title->c_str(), *x, *y, globalRect.width, globalRect.height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (!window)
            return;

        id = SDL_GetWindowID(window);

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        context = SDL_GL_CreateContext(window);
        String version = "300 es";
#else
        inject<Config> config;
        auto oglMajor = config->properties->get<int>("OpenGLMajor");
        auto oglMinor = config->properties->get<int>("OpenGLMinor");
        auto oglProfile = config->properties->get<std::string>("OpenGLProfile");
        if (oglMajor == 0) {
            oglMajor = 3;
            oglMinor = 3;
            oglProfile = "core";
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, oglMajor);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, oglMinor);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, oglProfile == "es" ? SDL_GL_CONTEXT_PROFILE_ES : SDL_GL_CONTEXT_PROFILE_CORE);
        context = SDL_GL_CreateContext(window);
        String version = std::to_string(oglMajor) + std::to_string(oglMinor) + "0 " + oglProfile;
        logV("Creating OGL context: ", version, " ", context ? "Success" : "Failed");
        if (!context) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            context = SDL_GL_CreateContext(window);
            version = "310 es";
            logV("Creating OGL context: ", version, " ", context ? "Success" : "Failed");
        }
#endif
        if (!context) {
            logE("Could not create OpenGL context");
            return;
        }

        auto profilePath = inject<Config>{}->properties->get<String>("icc-profile");
        if (!profilePath.empty()) {
            profile = inject<FileSystem>{}->parse(profilePath);
        }

        SDL_GL_MakeCurrent(window, context);
        graphics->init(version);
        resize();
        setDirty();

        plugins = NativeWindowPlugin::createAll();

        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(window, &wmInfo);

        for (auto& entry : plugins) {

#if defined(__WINDOWS__)
            entry.second->window = wmInfo.info.win.window;
            entry.second->hdc = wmInfo.info.win.hdc;
            entry.second->hinstance = wmInfo.info.win.hinstance;
#elif defined(__linux__) && !defined(ANDROID)
            if (wmInfo.subsystem == SDL_SYSWM_X11) {
                entry.second->display = wmInfo.info.x11.display;
                entry.second->window = wmInfo.info.x11.window;
            }
#endif

            entry.second->init();
        }
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
        graphics->alpha = 1.0f;
        graphics->scale = scale;
        graphics->begin(globalRect, *background);
        ui::Window::draw(z, *graphics.get());
        graphics->end();
        if (profile) {
            if (auto surface = graphics->read()) {
               if (profile->apply(surface))
                    graphics->write();
            }
        }
        SDL_GL_SwapWindow(window);
    }

    ~SDL2Window() {
        plugins.clear();

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

#endif
