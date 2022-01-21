// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <tools/Tool.hpp>

class InputColor : public ui::Controller {
    PubSub<msg::ActivateColor> pub{this};
    std::shared_ptr<ui::Node> picker;
    Property<bool> alwaysUpdate{this, "alwaysupdate", false};

public:
    void attach() override {
        node()->addEventListener<ui::Click>(this);
        if (alwaysUpdate)
            node()->set("value", Tool::color);
    }

    void eventHandler(const ui::Click&) {
        if (picker && !picker->getParent())
            picker.reset();
        if (!picker)
            picker = ui::Node::fromXML("colorpicker");
        picker->bringToFront();
    }

    void on(msg::ActivateColor& event) {
        if (picker || alwaysUpdate) {
            node()->set("value", event.color);
        }
    }
};

static ui::Controller::Shared<InputColor> inputcolor{"inputcolor"};
