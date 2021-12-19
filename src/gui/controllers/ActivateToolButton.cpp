// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <tools/Tool.hpp>

class ActivateToolButton : public ui::Controller {
public:
    Property<String> tool{this, "tool"};

    void attach() override {
        node()->addEventListener<ui::Click>(this);
    }

    void eventHandler(const ui::Click& event) {
        auto it = Tool::instances.find(*tool);
        if (it != Tool::instances.end()) {
            Tool::active = it->second;
        }
    }
};

static ui::Controller::Shared<ActivateToolButton> button{"activatetool"};
