// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "log/Log.hpp"
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Canvas : public ui::Controller {
public:
    std::shared_ptr<Surface> surface = std::make_shared<Surface>();

    Canvas() {
        surface->resize(128, 32);
        memset(surface->data(), 0xFF, surface->dataSize());
    }

    void attach(ui::Node* node) override {
        ui::Controller::attach(node);
        node->addEventListener<ui::MouseMove, ui::MouseDown>(this);
        node->init({{
                    {"surface", surface}
                }});
    }

    void eventHandler(const ui::MouseDown& event) {
        paint(event.targetX(), event.targetY());
    }

    void eventHandler(const ui::MouseMove& event) {
        if (event.buttons)
            paint(event.targetX(), event.targetY());
    }

    void paint(S32 tx, S32 ty) {
        S32 x = (tx * surface->width()) / node()->localRect.width;
        S32 y = (ty * surface->height()) / node()->localRect.height;
        surface->setPixel(x, y, Color{0xFF, 0, 0, 0xFF});
    }
};

static ui::Controller::Shared<Canvas> canvas{"canvas"};
