// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Button : public ui::Controller {
    PubSub<msg::Flush> pub{this};

public:
    Property<String> state{this, "state", "enabled", &Button::changeState};
    void changeState() {
        if (*state == "pressed" || *state == "active")
            node()->set("surface", *pressedSurface);
        if (*state == "enabled")
            node()->set("surface", *normalSurface);
        if (*state == "disabled")
            node()->set("surface", *disabledSurface);
    }

    Property<String> pressed{this, "down-src", "", &Button::reloadPressed};
    Property<std::shared_ptr<Surface>> pressedSurface{this, "down-surface"};
    void reloadPressed() {
        *pressedSurface = FileSystem::parse(*pressed);
    }

    Property<String> normal{this, "up-src", "", &Button::reloadNormal};
    Property<std::shared_ptr<Surface>> normalSurface{this, "up-surface"};
    void reloadNormal() {
        *normalSurface = FileSystem::parse(*normal);
        node()->set("surface", *normalSurface);
    }

    Property<String> disabled{this, "disabled-src", "", &Button::reloadDisabled};
    Property<std::shared_ptr<Surface>> disabledSurface{this, "disabled-surface"};
    void reloadDisabled() {
        *disabledSurface = FileSystem::parse(*disabled);
    }

    void on(msg::Flush& flush) {
        flush.hold(*pressedSurface);
        flush.hold(*normalSurface);
        flush.hold(*disabledSurface);
    }

    void attach() override {
        node()->addEventListener<ui::MouseDown, ui::MouseUp>(this);
    }

    void eventHandler(const ui::MouseDown& event) {
        if (*state == "enabled") {
            node()->set("state", "pressed");
            set("state", "pressed");
        }
    }

    void eventHandler(const ui::MouseUp& event) {
        if (*state == "pressed") {
            node()->set("state", "enabled");
            set("state", "enabled");
        }
    }
};

static ui::Controller::Shared<Button> button{"button"};
