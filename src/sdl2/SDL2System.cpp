// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <SDL2/SDL.h>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/System.hpp>
#include <gui/Events.hpp>
#include <log/Log.hpp>

class SDL2System : public System {
public:
    Provides sys{this};
    ui::Node::Provides win{"sdl2Window", "window"};
    PubSub<> pub{this};

    bool running = true;
    std::shared_ptr<ui::Node> root;
    ui::Node::Provides _root{root, "root"};

    bool boot() override {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            logE(SDL_GetError());
            return false;
        }
        root = inject<ui::Node>{"node"};
        root->processEvent(ui::AddToScene{});
        return true;
    }

    bool run() override {
        if (!running) return false;
        if (!root || root->getChildren().empty()) return false;
        pumpEvents();
        if (!running) return false;
        root->update();
        Graphics gfx;
        root->draw(0, gfx);
        return running;
    }

    void pumpEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
                pub(msg::WindowMaximized{event.window.windowID});
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                pub(msg::WindowMinimized{event.window.windowID});
                break;
            case SDL_WINDOWEVENT_RESTORED:
                pub(msg::WindowMinimized{event.window.windowID});
                break;
            case SDL_MOUSEMOTION:
                pub(msg::MouseMove{event.motion.windowID, event.motion.x, event.motion.y, event.motion.state});
                break;
            case SDL_MOUSEBUTTONUP:
                pub(msg::MouseUp{event.button.windowID, event.button.x, event.button.y, 1U << event.button.button});
                break;
            case SDL_MOUSEBUTTONDOWN:
                pub(msg::MouseDown{event.button.windowID, event.button.x, event.button.y, 1U << event.button.button});
                break;
            }
        }
    }

    ~SDL2System() {
        SDL_Quit();
    }
};

System::Shared<SDL2System> sys{"sdl2"};
