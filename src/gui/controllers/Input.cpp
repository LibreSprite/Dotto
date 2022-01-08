// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <cstddef>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Input : public ui::Controller {
    PubSub<> pub{this};

public:
    std::size_t cursorPosition = 0;
    Property<String> value{this, "value", ""};
    Property<String> text{this, "text", ""};
    Property<String> spanId{this, "span", "value"};
    std::shared_ptr<ui::Node> span;

    Property<String> state{this, "state", "enabled", &Input::changeState};
    void changeState() {
    }

    void attach() override {
        node()->addEventListener<ui::Focus, ui::Blur, ui::KeyDown, ui::KeyUp>(this);
        span = node()->findChildById(spanId);
    }

    void eventHandler(const ui::Focus&) {
        cursorPosition = value->size();
        String text = this->value;
        text.insert(cursorPosition, "∣");
        node()->set("text", text);
    }

    void eventHandler(const ui::Blur&) {
        node()->set("text", *value);
    }

    void eventHandler(const ui::KeyDown& event) {
        String keyName = event.keyname;
        String text = this->value;
        cursorPosition = std::min(text.size(), cursorPosition);
        if (keyName == "BACKSPACE") {
            if (!cursorPosition)
                return;
            text.erase(cursorPosition - 1, 1);
            cursorPosition--;
        } else if (keyName == "RIGHT") {
            cursorPosition = std::min(text.size(), cursorPosition + 1);
        } else if (keyName == "LEFT") {
            if (cursorPosition)
                cursorPosition--;
        } else if (event.keycode >= ' ' && event.keycode < 0x80) {
            String key(reinterpret_cast<const char*>(&event.keycode));
            text.insert(cursorPosition++, key, 0, key.size());
        } else {
            return;
        }
        node()->set("value", text);
        text.insert(cursorPosition, "∣");
        node()->set("text", text);
    }

    void eventHandler(const ui::KeyUp& event) {

    }
};

static ui::Controller::Shared<Input> input{"input"};
