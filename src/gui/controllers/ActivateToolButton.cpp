// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <tools/Tool.hpp>

class ActivateToolButton : public ui::Controller {
public:
    Property<String> tool{this, "tool"};
    PubSub<msg::ActivateTool> pub{this};

    void attach() override {
        node()->addEventListener<ui::Click>(this);
    }

    void on(const msg::ActivateTool& event) {
        node()->load({{"state", event.tool == *tool ? "active" : "enabled"}});
    }

    void eventHandler(const ui::Click& event) {
        if (auto command = inject<Command>{"activatetool"}) {
            command->load(node()->getPropertySet());
            command->run();
        }
    }
};

static ui::Controller::Shared<ActivateToolButton> button{"activatetool"};
