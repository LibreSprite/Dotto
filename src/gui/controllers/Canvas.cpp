// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <tools/Tool.hpp>

class Canvas : public ui::Controller {
public:
    std::shared_ptr<Surface> surface = std::make_shared<Surface>();
    std::shared_ptr<Tool> activeTool;
    Tool::Path points;

    Canvas() {
        surface->resize(128, 128);
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
        end();
        paint(event.targetX(), event.targetY());
    }

    void eventHandler(const ui::MouseMove& event) {
        if (event.buttons) {
            paint(event.targetX(), event.targetY());
        } else {
            end();
        }
    }

    void end() {
        if (points.empty())
            return;
        if (activeTool)
            activeTool->end(surface.get(), points);
        points.clear();
    }

    void paint(S32 tx, S32 ty) {
        S32 x = (tx * surface->width()) / node()->localRect.width;
        S32 y = (ty * surface->height()) / node()->localRect.height;
        bool begin = points.empty();

        points.push_back({x, y});

        if (begin) {
            activeTool = Tool::active.lock();
            if (!activeTool)
                return;
            activeTool->begin(surface.get(), points);
        } else if (activeTool) {
            activeTool->update(surface.get(), points);
        }
    }
};

static ui::Controller::Shared<Canvas> canvas{"canvas"};
