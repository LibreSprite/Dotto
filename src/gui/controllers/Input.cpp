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
#include <regex>

class Input : public ui::Controller {
    PubSub<> pub{this};

public:
    std::size_t cursorPosition = 0;
    Property<String> allowRegex{this, "allow", ""};
    Property<String> text{this, "text", ""};
    Property<String> spanId{this, "span", "value"};
    std::shared_ptr<ui::Node> span;

    Property<String> state{this, "state", "enabled", &Input::changeState};
    void changeState() {
    }

    void attach() override {
        node()->addEventListener<ui::MouseDown, ui::KeyDown, ui::KeyUp>(this);
        span = node()->findChildById(spanId);
    }

    void eventHandler(const ui::MouseDown& event) {
        if (!span)
            return;

        auto advanceValue = span->get("text-advance");
        if (!advanceValue || !advanceValue->has<Vector<S32>>())
            return;

        auto advance = advanceValue->get<Vector<S32>>();
        S32 x = event.globalX - span->globalRect.x;
        S32 cursorX = 0;
        S32 nextCursorX = 0;
        cursorPosition = 0;
        for (U32 i = 0; i < advance.size(); ++i) {
            nextCursorX += advance[i];
            if (std::abs(x - cursorX) > std::abs(x - nextCursorX)) {
                cursorPosition = i + 1;
                cursorX = nextCursorX;
            }
        }

        span->set("text", ""); // force redraw to get rid of caret
        span->set("text", *text);
        auto surfaceValue = span->get("surface");
        if (surfaceValue && surfaceValue->has<std::shared_ptr<Surface>>()) {
            drawCaret(surfaceValue->get<std::shared_ptr<Surface>>(), cursorX);
        }
    }

    void drawCaret(std::shared_ptr<Surface> surface, S32 cursorX) {
        if (surface) {
            for (S32 y = 0; y < surface->height(); ++y) {
                surface->setPixel(cursorX, y, Color{0,0,0});
            }
        }
    }

    void eventHandler(const ui::KeyDown& event) {
        String keyName = event.keyname;
        String text = this->text;
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
            if (!allowRegex->empty() && !std::regex_match(key, std::regex(*allowRegex))) {
                return;
            }
            text.insert(cursorPosition++, key, 0, key.size());
        } else {
            return;
        }

        if (span)
            span->set("text", ""); // force redraw to get rid of caret
        node()->set("text", text);
        node()->set("value", text);

        if (span) {
            auto advanceValue = span->get("text-advance");
            auto surfaceValue = span->get("surface");
            if (advanceValue && advanceValue->has<Vector<S32>>() &&
                surfaceValue && surfaceValue->has<std::shared_ptr<Surface>>()) {
                auto advance = advanceValue->get<Vector<S32>>();
                if (auto surface = surfaceValue->get<std::shared_ptr<Surface>>()) {
                    auto end = std::min(cursorPosition, advance.size());
                    S32 cursorX = 0;
                    for (std::size_t i = 0; i < end; ++i) {
                        cursorX += advance[i];
                    }
                    drawCaret(surface, cursorX);
                }
            }
        }
    }

    void eventHandler(const ui::KeyUp& event) {

    }
};

static ui::Controller::Shared<Input> input{"input"};
