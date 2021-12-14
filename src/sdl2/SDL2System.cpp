// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <SDL2/SDL.h>

#include <common/System.hpp>
#include <gui/Events.hpp>
#include <log/Log.hpp>

class SDL2System : public System {
public:
    Provides sys{this};
    ui::Node::Provides win{"sdl2Window", "window"};

    bool running = true;
    std::shared_ptr<ui::Node> root;

    bool boot() override {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            logE(SDL_GetError());
            return false;
        }
        root = inject<ui::Node>{"node"};
        root->processEvent(ui::AddToScene{});
        return true;
    }

    std::shared_ptr<ui::Window> openWindow(const PropertySet& properties) override {
        std::shared_ptr<ui::Window> window = inject<ui::Node>{"window"};
        if (window) {
            window->init(properties);
            root->addChild(window);

            PropertySet imgProp;
            imgProp.set("src", "%userhome/Pictures/2021-08-16_12-49.png");
            std::shared_ptr<ui::Node> img = inject<ui::Node>{"image"};
            img->init(imgProp);
            window->addChild(img);
        }
        return window;
    }

    bool run() override {
        if (!running) return false;
        if (!root || root->getChildren().empty()) return false;
        pumpEvents();
        if (!running) return false;
        root->update();
        root->draw(0);
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
