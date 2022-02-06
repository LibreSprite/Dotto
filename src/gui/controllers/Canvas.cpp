// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <tools/Tool.hpp>

class Canvas : public ui::Controller {
public:
    PubSub<msg::ModifyCell, msg::ActivateCell> pub{this};
    inject<Cell> cell;
    std::shared_ptr<Tool> activeTool;
    Tool::Path points;

    void attach() override {
        node()->addEventListener<ui::MouseMove,
                                 ui::MouseDown,
                                 ui::MouseUp,
                                 ui::MouseWheel>(this);
        setup();
    }

    void on(msg::ModifyCell& event) {
        if (event.cell.get() == cell)
            setup();
    }

    void on(msg::ActivateCell& event) {
        node()->set("inputEnabled", event.cell.get() == cell);
    }

    void setup() {
        auto surface = cell->getComposite()->shared_from_this();
        if (!surface) {
            logI("Empty cell");
        }
        node()->load({
                {"surface", surface},
                {"alpha", cell->getAlpha()}
            });
    }

    void eventHandler(const ui::MouseDown& event) {
        end();
        paint(event.targetX(), event.targetY(), event.buttons);
    }

    void eventHandler(const ui::MouseUp& event) {
        end();
    }

    void eventHandler(const ui::MouseMove& event) {
        if (event.buttons) {
            paint(event.targetX(), event.targetY(), event.buttons);
        } else {
            end();
        }
    }

    void eventHandler(const ui::MouseWheel& event) {
        inject<Command> zoom{"zoom"};
        zoom->set("level", "*" + tostring(1 + 0.2 * event.wheelY));
        zoom->run();
    }

    void end() {
        if (points.empty())
            return;
        if (activeTool)
            activeTool->end(cell->getComposite(), points);
        points.clear();
    }

    void paint(S32 tx, S32 ty, U32 buttons) {
        auto rect = node()->globalRect;
        if (!rect.width || !rect.height)
            return;

        auto surface = cell->getComposite();
        S32 x = (tx * surface->width()) / rect.width;
        S32 y = (ty * surface->height()) / rect.height;
        bool begin = points.empty();

        points.push_back({x, y});

        if (begin) {
            do {
                activeTool = Tool::active.lock();
                if (!activeTool)
                    return;
                activeTool->begin(surface, points, buttons);
            } while (Tool::active.lock() != activeTool);
        } else if (activeTool) {
            activeTool->update(surface, points);
        }
    }
};

static ui::Controller::Shared<Canvas> canvas{"canvas"};
