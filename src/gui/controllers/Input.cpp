// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Font.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <cstddef>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <regex>

static const char* cursor = "\x1B[zw]|\x1B[w]";
static const Vector<Font::Entity> pcursor = Font::parse(cursor);

class Input : public ui::Controller {
    PubSub<> pub{this};

public:
    Property<String> allowRegex{this, "allow", ""};
    Property<String> text{this, "text", ""};
    Property<String> spanId{this, "span", "value"};
    Property<String> value{this, "value", "", &Input::changeValue};
    ui::Node* span = nullptr;

    void changeValue() {
        node()->set("text", *value);
    }

    void attach() override {
        node()->addEventListener<ui::MouseDown,
                                 ui::TextEvent,
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
        auto entities = Font::parse(*this->text);
        U32 offset = 0;
        bool didClear = false;
        for (auto pos : carets(entities)) {
            auto begin = entities.begin() + (pos - offset);
            entities.erase(begin, begin + pcursor.size());
            offset += pcursor.size();
            didClear = true;
        }
        if (didClear) {
            node()->set("text", Font::toString(entities, false));
        }
    }

    void eventHandler(const ui::Blur&) {
        clearCaret();
    }

    void eventHandler(const ui::Focus& event) {
        auto entities = Font::parse(*text);
        if (carets(entities).empty()) {
            entities.insert(entities.end(), pcursor.begin(), pcursor.end());
            node()->set("text", Font::toString(entities, false));
        }
    }

    void eventHandler(const ui::MouseDown& event) {
        if (!span)
            return;

        clearCaret();
        auto advanceValue = span->get("text-advance");
        if (!advanceValue || !advanceValue->has<Vector<S32>>())
            return;

        auto advance = advanceValue->get<Vector<S32>>();
        auto entities = Font::parse(*this->text);
        S32 x = event.globalX - span->globalRect.x;
        S32 cursorX = 0;
        S32 nextCursorX = 0;
        U32 cursorPosition = 0;
        U32 advanceIndex = 0;
        for (U32 i = 0, max = entities.size(); i < max; ++i) {
            if (!std::get_if<U32>(&entities[i])) {
                continue;
            }

            while (advanceIndex < advance.size() && !advance[advanceIndex]) {
                advanceIndex++;
            }

            if (advanceIndex >= advance.size()) {
                break;
            }

            nextCursorX += advance[advanceIndex++];
            if (std::abs(x - cursorX) > std::abs(x - nextCursorX)) {
                cursorPosition = i + 1;
                cursorX = nextCursorX;
            }
        }
        entities.insert(entities.begin() + cursorPosition, pcursor.begin(), pcursor.end());
        node()->set("text", Font::toString(entities, false));
    }

    Vector<U32> carets(const Vector<Font::Entity>& entities) {
        Vector<U32> pos;
        if (entities.size() < pcursor.size()) {
            return pos;
        }
        for (U32 i = 0, max = entities.size() - pcursor.size(); i <= max; ++i) {
            bool match = true;
            for (U32 j = 0, jmax = pcursor.size(); match && j < jmax; ++j) {
                match = entities[i + j] == pcursor[j];
            }
            if (match) {
                pos.push_back(i);
                i += pcursor.size() - 1;
            }
        }
        return pos;
    }

    void addBeforeCaret(std::string_view key) {
        auto entities = Font::parse(*this->text);
        auto insert = Font::parse(key);
        U32 offset = 0;
        for (auto pos : carets(entities)) {
            entities.insert(std::next(entities.begin(), offset + pos), insert.begin(), insert.end());
            offset += insert.size();
        }
        node()->set("text", Font::toString(entities, false));
        node()->set("value", *value = Font::toString(entities, true));
    }

    void eventHandler(const ui::TextEvent& event) {
        if (event.pressedKeys.count("LCTRL") || event.pressedKeys.count("RCTRL"))
            return;

        std::string_view key = event.text;
        if (key.empty() || key[0] < ' ')
            return;

        try {
            if (!allowRegex->empty() && !std::regex_match(event.text, std::regex(*allowRegex))) {
                return;
            }
        } catch (std::regex_error& err) {
            logE("Input Regex Error: ", err.what(), "\nExpression: /", *allowRegex, "/");
        }

        addBeforeCaret(key);
        node()->processEvent(ui::Changed{node()});
    }

    void eventHandler(const ui::KeyDown& event) {
        if (event.pressedKeys.count("LCTRL") || event.pressedKeys.count("RCTRL")) {
            return;
        }
        event.cancel = true;
        String keyName = event.keyname;
        auto keycode = event.keycode;
        auto entities = Font::parse(*this->text);
        if (startsWith(keyName, "KP_")) {
            keycode = keyName[3];
            keyName = keyName.substr(3);
        }
        if (keyName == "BACKSPACE") {
            bool changed = false;
            auto positions = carets(entities);
            for (S32 i = positions.size() - 1; i > -1; --i) {
                auto pos = positions[i];
                auto it = entities.begin() + pos;
                while (--pos < entities.size() && !std::get_if<U32>(&entities[pos]));
                if (pos >= entities.size()) {
                    event.cancel = false;
                    break;
                }
                entities.erase(entities.begin() + pos);
                changed = true;
            }
            if (changed) {
                node()->set("text", Font::toString(entities, false));
                node()->set("value", *value = Font::toString(entities, true));
                node()->processEvent(ui::Changed{node()});
            }
        } else if (keyName == "RIGHT") {
            auto positions = carets(entities);
            for (S32 i = positions.size() - 1; i > -1; --i) {
                auto pos = positions[i];
                auto it = entities.begin() + pos;
                entities.erase(it, it + pcursor.size());
                while (pos < entities.size() && !std::get_if<U32>(&entities[pos])) {
                    pos++;
                }
                if (pos >= entities.size()) {
                    event.cancel = false;
                    pos = entities.size() - 1;
                }
                entities.insert(entities.begin() + (pos + 1), pcursor.begin(), pcursor.end());
            }
            node()->set("text", Font::toString(entities, false));
        } else if (keyName == "LEFT") {
            for (auto pos : carets(entities)) {
                auto it = entities.begin() + pos;
                entities.erase(it, it + pcursor.size());
                while (--pos < entities.size() && !std::get_if<U32>(&entities[pos]));
                if (pos >= entities.size()) {
                    event.cancel = false;
                    pos = 0;
                }
                entities.insert(entities.begin() + pos, pcursor.begin(), pcursor.end());
            }
            node()->set("text", Font::toString(entities, false));
        } else if (keycode >= ' ' && keycode < 0x80) {
            return;
        } else {
            event.cancel = false;
            return;
        }
    }

    void eventHandler(const ui::KeyUp& event) {
        event.cancel = true;
    }
};

static ui::Controller::Shared<Input> input{"input"};
