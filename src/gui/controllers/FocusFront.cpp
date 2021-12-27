// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class FocusFront : public ui::Controller {
public:
    void attach() override {
        node()->addEventListener<ui::MouseDown>(this);
    }

    void eventHandler(const ui::MouseDown& event) {
        node()->getParent()->bringToFront(node()->shared_from_this());
    }
};

static ui::Controller::Shared<FocusFront> ff{"focusfront"};
