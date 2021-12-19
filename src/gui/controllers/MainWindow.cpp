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
    void attach(ui::Node* node) override {
        ui::Controller::attach(node);
        Tool::boot();
        node->addEventListener<ui::KeyDown, ui::KeyUp>(this);
        // node->init({{
        //             {"surface", surface}
        //         }});
    }

    void eventHandler(const ui::KeyDown& event) {
        if (event.keycode == 'b') {
            Tool::active = Tool::instances.back().shared();
        } else if (event.keycode == 'f') {
            Tool::active = Tool::instances.front().shared();
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
