// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Window.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class InputColor : public ui::Controller {
    PubSub<msg::ActivateColor> pub{this};
    Property<String> pickerId{this, "picker-id", "colorpanel"};
    Property<String> hide{this, "hide", ""};
    Property<bool> alwaysUpdate{this, "alwaysupdate", false};
    InputColor* activeInput = nullptr;

public:
    ~InputColor() {
        if (activeInput == this)
            activeInput = nullptr;
    }

    void attach() override {
        node()->addEventListener<ui::Click>(this);
        String color;
        if (alwaysUpdate) {
            color = Tool::color.toString();
        } else {
            color = node()->getPropertySet().get<Color>("value");
        }

        node()->set("value", color);
    }

    void eventHandler(const ui::Click&) {
        String mode = "toggle";
        if (activeInput != this) {
            inject<Command> activate{"activatecolor"};
            activate->set("color", node()->getPropertySet().get<Color>("value"));
            activate->run();
            mode = "set";
        }

        activeInput = this;

        if (auto toggleui = inject<Command>{"toggleui"}) {
            toggleui->load({
                    {"id", *pickerId},
                    {"hide", *hide},
                    {"mode", mode}
                });
            toggleui->run();
        }
    }

    void on(msg::ActivateColor& event) {
        if (alwaysUpdate || activeInput == this) {
            node()->set("value", event.color.toString());
            node()->processEvent(ui::Changed{node()});
        }
    }
};

static ui::Controller::Shared<InputColor> inputcolor{"inputcolor"};
