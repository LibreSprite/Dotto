// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "common/Messages.hpp"
#include "doc/Cell.hpp"
#include "tools/Tool.hpp"
#include <layer/Layer.hpp>
#include <common/System.hpp>

class BitmapLayer : public Layer {
    PubSub<msg::ActivateTool> pub{this};
    U32 prevButtons = ~U32{};
    std::shared_ptr<Tool> activeTool;
    Tool::Path points;
    inject<System> system;

public:
    void on(msg::ActivateTool&) {end();}

    void update() override {
        if (prevButtons != buttons())
            end();

        prevButtons = buttons();
        if (globalCanvas().empty())
            return;

        auto point = localMouse();

        auto surface = cell().getComposite();
        bool begin = points.empty();
        if (!begin && point.x == points.back().x && point.y == points.back().y)
            return;
        points.push_back(point);

        if (begin) {
            do {
                activeTool = Tool::active.lock();
                if (!activeTool)
                    return;
                activeTool->begin(surface, points, buttons());
            } while (Tool::active.lock() != activeTool);
        } else if (activeTool) {
            activeTool->update(surface, points);
        }
    }

    void end() {
        prevButtons = ~U32{};
        if (points.empty())
            return;
        if (activeTool)
            activeTool->end(cell().getComposite(), points);
        points.clear();
    }
};

static Layer::Shared<BitmapLayer> reg{"bitmap"};
