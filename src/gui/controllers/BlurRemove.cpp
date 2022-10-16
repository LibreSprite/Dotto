// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class BlurRemove : public ui::Controller {
public:
    void attach() override {
        node()->addEventListener<ui::Blur, ui::BlurChild>(this);
    }

    void eventHandler(const ui::Blur&) {
        if (auto focus = node()->getFocus(); !focus || !focus->isDescendantOf(node()->shared_from_this()))
            node()->remove();
    }

    void eventHandler(const ui::BlurChild&) {
        if (auto focus = node()->getFocus(); !focus || !focus->isDescendantOf(node()->shared_from_this()))
            node()->remove();
    }
};

static ui::Controller::Shared<BlurRemove> blurremove{"blur-remove"};
