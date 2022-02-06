// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <log/Log.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Debug : public ui::Controller {
public:
    void attach() override {
        node()->addEventListener<ui::AddToScene>(this);
    }

    void eventHandler(const ui::AddToScene&) {
        logI("UI Node Dump");
        for (auto& entry : node()->getPropertySet().getMap()) {
            logI("[", entry.first, "] = [", entry.second->toString(), "]");
        }
    }
};

static ui::Controller::Shared<Debug> debug{"debug"};
