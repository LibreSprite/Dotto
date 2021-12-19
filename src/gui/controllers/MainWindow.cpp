// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class MainWindow : public ui::Controller {
public:
    void attach() override {
        Tool::boot();
        node()->addEventListener<ui::KeyDown, ui::KeyUp>(this);
    }

    void eventHandler(const ui::KeyDown& event) {
        if (event.keycode == 'b') {
            Tool::active = Tool::instances["pencil"];
        } else if (event.keycode == 'f') {
            Tool::active = Tool::instances["bucket"];
        }
        if (auto tool = Tool::active.lock())
            logV("Active tool: ", tool->getName());
        else
            logV("Event keycode: ", event.keycode);
    }

    void eventHandler(const ui::KeyUp& event) {
    }
};

static ui::Controller::Shared<MainWindow> mainwindow{"main"};
