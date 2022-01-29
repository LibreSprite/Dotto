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

class CheckBox : public ui::Controller {
    PubSub<msg::Flush> pub{this};

public:
    Property<String> value{this, "value", "unchecked", &CheckBox::changeState};

    void changeState() {
        std::shared_ptr<Surface> current;
        Color multiply;
        if (*value == "true") {
            current = trueSurface;
            multiply = trueMultiply;
        } else {
            current = falseSurface;
            multiply = falseMultiply;
        }
        if (multiply.a == 0 && current)
            multiply = 0xFFFFFFFF;
        node()->set("surface", current);
        node()->set("multiply", multiply);
    }

    Property<String> trueSrc{this, "true-src", "", &CheckBox::reloadTrue};
    Property<std::shared_ptr<Surface>> trueSurface{this, "true-surface"};
    void reloadTrue() {
        *trueSurface = FileSystem::parse(*trueSrc);
        changeState();
    }

    Property<String> falseSrc{this, "false-src", "", &CheckBox::reloadFalse};
    Property<std::shared_ptr<Surface>> falseSurface{this, "false-surface"};
    void reloadFalse() {
        *falseSurface = FileSystem::parse(*falseSrc);
        changeState();
    }

    Property<Color> trueMultiply{this, "true-multiply", {0,0,0,0}, &CheckBox::changeState};
    Property<Color> falseMultiply{this, "false-multiply", {0,0,0,0}, &CheckBox::changeState};

    Property<FunctionRef<void()>, true> clickCallback{this, "click"};

    void on(msg::Flush& flush) {
        flush.hold(*trueSurface);
        flush.hold(*falseSurface);
    }

    void attach() override {
        node()->addEventListener<ui::MouseDown,
                                 ui::MouseUp,
                                 ui::MouseEnter,
                                 ui::MouseLeave,
                                 ui::Click,
                                 ui::KeyDown,
                                 ui::KeyUp>(this);
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
        if (*value != "true") {
            node()->set("value", "true");
            set("value", "true");
        } else {
            node()->set("value", "false");
            set("value", "false");
        }
        node()->processEvent(ui::Changed{node()});
    }

    void eventHandler(const ui::MouseUp&) {
    }

    void eventHandler(const ui::MouseEnter&) {
    }

    void eventHandler(const ui::MouseLeave&) {
    }
};

static ui::Controller::Shared<CheckBox> checkbox{"checkbox"};
