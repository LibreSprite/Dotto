// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Popup : public ui::Controller {
    std::shared_ptr<ui::Node> closeButton;
public:
    void attach() override {
        node()->addEventListener<ui::Click, ui::KeyUp>(this);
        closeButton = node()->findChildById("close");
    }

    void eventHandler(const ui::KeyUp& event) {
        if (String(event.keyname) == "ESCAPE" && closeButton) {
            node()->remove();
        }
    }

    void eventHandler(const ui::Click& event) {
        if (event.target->isDescendantOf(closeButton)) {
            node()->remove();
        }
    }
};

static ui::Controller::Shared<Popup> popup{"popup"};
