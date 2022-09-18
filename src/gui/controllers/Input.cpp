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
    ui::Node* span = nullptr;

    Property<String> state{this, "state", "enabled", &Input::changeState};
    void changeState() {
    }

    void attach() override {
        node()->addEventListener<ui::MouseDown,
                                 ui::KeyDown,
                                 ui::KeyUp,
                                 ui::Blur,
                                 ui::Focus>(this);
        span = node()->findChildById(spanId).get();
        if (span)
            span->set("inputEnabled", false);
        else
            span = node();
    }

    void clearCaret() {
        if (span) {
            auto backup = *text;
            span->set("text", ""); // force redraw to get rid of caret
            span->set("text", backup);
        }
    }

    void eventHandler(const ui::Blur&) {
        clearCaret();
    }

    void eventHandler(const ui::Focus& event) {
        auto index = span->get("cursor-index");
        cursorPosition = std::min<U32>(text->size(), index ? index->get<U32>() : ~U32{});
        clearCaret();
        drawCaret();
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

        node()->set("cursor-index", cursorPosition);
        clearCaret();
        drawCaret();
    }

    void drawCaret(std::shared_ptr<Surface> surface, S32 cursorX) {
        if (surface) {
            Color color;
            if (span)
                color = span->getPropertySet().get<Color>("color");
            surface->setVLine(cursorX, 0, surface->height(), color.toU32());
        }
    }

    void eventHandler(const ui::KeyDown& event) {
        if (event.pressedKeys.count("LCTRL") || event.pressedKeys.count("RCTRL")) {
            return;
        }
        bool changed = false;
        event.cancel = true;
        String keyName = event.keyname;
        auto keycode = event.keycode;
        String text = this->text;
        cursorPosition = std::min(text.size(), cursorPosition);
        if (startsWith(keyName, "KP_")) {
            keycode = keyName[3];
            keyName = keyName.substr(3);
        }
        if (keyName == "BACKSPACE") {
            if (!cursorPosition) {
                event.cancel = false;
                return;
            }
            changed = true;
            text.erase(cursorPosition - 1, 1);
            cursorPosition--;
        } else if (keyName == "RIGHT") {
            if (cursorPosition == text.size()) {
                event.cancel = false;
                return;
            }
            cursorPosition = std::min(text.size(), cursorPosition + 1);
        } else if (keyName == "LEFT") {
            if (cursorPosition) {
                cursorPosition--;
            } else {
                event.cancel = false;
                return;
            }
        } else if (keycode >= ' ' && keycode < 0x80) {
            String key(reinterpret_cast<const char*>(&keycode));
            if (event.pressedKeys.count("LSHIFT") || event.pressedKeys.count("RSHIFT")) {
                key[0] = std::toupper(keycode);
            }
            try {
                if (!allowRegex->empty() && !std::regex_match(key, std::regex(*allowRegex))) {
                    return;
                }
            } catch (std::regex_error& err) {
                logE("Input Regex Error: ", err.what(), "\nExpression: /", *allowRegex, "/");
            }
            text.insert(cursorPosition++, key, 0, key.size());
            changed = true;
        } else {
            event.cancel = false;
            return;
        }

        if (span)
            span->set("text", ""); // force redraw to get rid of caret
        node()->set("text", text);
        node()->set("value", text);
        node()->set("cursor-index", cursorPosition);

        drawCaret();

        if (changed)
            node()->processEvent(ui::Changed{node()});
    }

    void drawCaret() {
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
                    cursorX = std::min<S32>(surface->width() - 1, cursorX);
                    drawCaret(surface, cursorX);
                }
            }
        }
    }

    void eventHandler(const ui::KeyUp& event) {
        event.cancel = true;
    }
};

static ui::Controller::Shared<Input> input{"input"};
