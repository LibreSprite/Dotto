// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/FunctionRef.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Button : public ui::Controller {
    PubSub<msg::Flush> pub{this};
    bool isHovering = false;

public:
    Property<String> state{this, "state", "enabled", &Button::changeState};
    void changeState() {
        std::shared_ptr<Surface> current;
        Color multiply;
        if (*state == "pressed" || *state == "active") {
            current = pressedSurface;
            multiply = pressedMultiply;
        } else if (*state == "enabled") {
            if (isHovering) {
                current = *hoverSurface ? hoverSurface : normalSurface;
                multiply = hoverMultiply;
            } else if (node()->hasFocus()) {
                current = *focusSurface ? focusSurface : normalSurface;
                multiply = focusMultiply;
            } else {
                current = normalSurface;
                multiply = normalMultiply;
            }
        } else if (*state == "disabled") {
            current = disabledSurface;
            multiply = disabledMultiply;
        }
        if (!current)
            current = allSurface;
        if (multiply.a == 0 && current)
            multiply = 0xFFFFFFFF;
        node()->set("surface", current);
        node()->set("multiply", multiply);
    }

    Property<String> allSrc{this, "src", "", &Button::reloadAllSrc};
    Property<std::shared_ptr<Surface>> allSurface{this, "all-surface"};
    void reloadAllSrc() {
        *allSurface = FileSystem::parse(*allSrc);
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

    Property<String> hover{this, "hover-src", "", &Button::reloadHover};
    Property<std::shared_ptr<Surface>> hoverSurface{this, "hover-surface"};
    void reloadHover() {
        *hoverSurface = FileSystem::parse(*hover);
    }

    Property<String> focus{this, "focus-src", "", &Button::reloadFocus};
    Property<std::shared_ptr<Surface>> focusSurface{this, "focus-surface"};
    void reloadFocus() {
        *focusSurface = FileSystem::parse(*focus);
    }

    Property<String> disabled{this, "disabled-src", "", &Button::reloadDisabled};
    Property<std::shared_ptr<Surface>> disabledSurface{this, "disabled-surface"};
    void reloadDisabled() {
        *disabledSurface = FileSystem::parse(*disabled);
    }

    Property<Color> pressedMultiply{this, "down-multiply", {0,0,0,0}};
    Property<Color> normalMultiply{this, "up-multiply", {0,0,0,0}, &Button::changeNormalColor};
    void changeNormalColor() {
        node()->set("multiply", *normalMultiply);
    }
    Property<Color> hoverMultiply{this, "hover-multiply", {0,0,0,0}};
    Property<Color> focusMultiply{this, "focus-multiply", {0,0,0,0}};
    Property<Color> disabledMultiply{this, "disabled-multiply", {0,0,0,0}};

    Property<FunctionRef<void()>> clickCallback{this, "click"};

    void on(msg::Flush& flush) {
        flush.hold(*allSurface);
        flush.hold(*pressedSurface);
        flush.hold(*normalSurface);
        flush.hold(*disabledSurface);
        flush.hold(*hoverSurface);
        flush.hold(*focusSurface);
    }

    void attach() override {
        node()->addEventListener<ui::MouseDown,
                                 ui::MouseUp,
                                 ui::MouseEnter,
                                 ui::MouseLeave,
                                 ui::Click,
                                 ui::KeyDown,
                                 ui::KeyUp,
                                 ui::Focus,
                                 ui::Blur>(this);
    }

    void eventHandler(const ui::Focus&) {
        changeState();
    }

    void eventHandler(const ui::Blur&) {
        changeState();
    }

    void eventHandler(const ui::KeyDown& event) {
        if (event.keyname == String("RETURN"))
            node()->processEvent(ui::MouseDown{node(), 0, 0, 1});
    }

    void eventHandler(const ui::KeyUp& event) {
        if (event.keyname == String("RETURN")) {
            node()->processEvent(ui::MouseUp{node(), 0, 0, 1});
            node()->processEvent(ui::Click{node(), 0, 0, 1});
        }
    }

    void eventHandler(const ui::Click&) {
        if (*clickCallback)
            (*clickCallback)();
    }

    void eventHandler(const ui::MouseDown&) {
        if (*state == "enabled") {
            node()->set("state", "pressed");
            set("state", "pressed");
        }
    }

    void eventHandler(const ui::MouseUp&) {
        if (*state == "pressed") {
            node()->set("state", "enabled");
            set("state", "enabled");
        }
    }

    void eventHandler(const ui::MouseEnter&) {
        isHovering = true;
        changeState();
    }

    void eventHandler(const ui::MouseLeave&) {
        isHovering = false;
        if (*state == "pressed") {
            node()->set("state", "enabled");
            set("state", "enabled");
        } else changeState();
    }
};

static ui::Controller::Shared<Button> button{"button"};
