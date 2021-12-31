// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Cell.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <tools/Tool.hpp>

class Canvas : public ui::Controller {
public:
    inject<Cell> cell{"temporary"};
    Cell::Provides tmp{cell.shared(), "activecell"};

    std::shared_ptr<Tool> activeTool;
    Tool::Path points;

    void attach() override {
        auto surface = cell->getComposite()->shared_from_this();
        node()->addEventListener<ui::MouseMove, ui::MouseDown, ui::MouseUp>(this);
        node()->set("surface", surface);
    }

    void eventHandler(const ui::MouseDown& event) {
        end();
        paint(event.targetX(), event.targetY());
    }

    void eventHandler(const ui::MouseUp& event) {
        end();
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
            activeTool->end(cell->getComposite(), points);
        points.clear();
    }

    void paint(S32 tx, S32 ty) {
        auto surface = cell->getComposite();
        S32 x = (tx * surface->width()) / node()->localRect.width;
        S32 y = (ty * surface->height()) / node()->localRect.height;
        bool begin = points.empty();

        points.push_back({x, y});

        if (begin) {
            do {
                activeTool = Tool::active.lock();
                if (!activeTool)
                    return;
                activeTool->begin(surface, points);
            } while (Tool::active.lock() != activeTool);
        } else if (activeTool) {
            activeTool->update(surface, points);
        }
    }
};

static ui::Controller::Shared<Canvas> canvas{"canvas"};
