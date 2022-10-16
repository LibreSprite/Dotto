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
    Property<F32> speed{this, "speed", 0.05f};
    Property<F32> minalpha{this, "min-alpha", 0.2f};
    Property<F32> maxalpha{this, "max-alpha", 1.0f};

    std::optional<PubSub<msg::Tick>> tickpub;
    bool hover = false;

    void attach() override {
        node()->addEventListener<ui::MouseEnter, ui::MouseLeave>(this);
        node()->set("alpha", *minalpha);
    }

    void eventHandler(const ui::MouseEnter&) {
        hover = true;
        tickpub.emplace(this);
    }

    void eventHandler(const ui::MouseLeave&) {
        hover = false;
        tickpub.emplace(this);
    }

    void on(msg::Tick&) {
        auto alpha = node()->getPropertySet().get<F32>("alpha");
        auto target = hover ? *maxalpha : *minalpha;
        auto delta = (target - alpha) * speed;
        alpha += delta;
        if (std::abs(delta) < 0.001) {
            tickpub.reset();
            alpha = target;
        }
        node()->set("alpha", alpha);
    }
};

static ui::Controller::Shared<HoverReveal> hoverreveal{"hover-reveal"};
