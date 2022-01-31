// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <optional>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Unit.hpp>

class HoverReveal : public ui::Controller {
public:
    PubSub<> pub{this};
    std::optional<PubSub<msg::Tick>> tickpub;
    bool hover = false;

    void attach() override {
        node()->addEventListener<ui::MouseEnter, ui::MouseLeave>(this);
    }

    void eventHandler(const ui::MouseEnter& event) {
        hover = true;
        tickpub.emplace(this);
    }

    void eventHandler(const ui::MouseLeave& event) {
        hover = false;
        tickpub.emplace(this);
    }

    void on(msg::Tick&) {
        auto alpha = node()->getPropertySet().get<F32>("alpha");
        auto delta = (hover ? 1.0f : 0.1f) - alpha;
        if (std::abs(delta) > 0.001) {
            alpha += delta * 0.03f;
            node()->set("alpha", alpha);
        } else {
            tickpub.reset();
        }
    }
};

static ui::Controller::Shared<HoverReveal> hoverreveal{"hover-reveal"};
