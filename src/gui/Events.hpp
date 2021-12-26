// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>

namespace ui {

    class Node;

    class Event {
    public:
        virtual ~Event() = default;

        mutable Node* target = nullptr;

        enum class Bubble {
            None,
            Up,
            Down
        };
        Bubble bubble = Bubble::None;

        bool cascade = true;
        mutable bool cancel = false;

        S32 globalX;
        S32 globalY;

        virtual S32 targetX() const;
        virtual S32 targetY() const;
        virtual Vector<String> toStrings(const String& name) const;
    };

    struct AddToScene : public Event {
        AddToScene() {
            bubble = Bubble::Down;
        }
    };

    struct Blur : public Event {};
    struct Focus : public Event {};

    struct RemoveFromScene : public Event {
        RemoveFromScene() {
            bubble = Bubble::Down;
        }
    };

    struct MouseEnter : public Event {};

    struct MouseLeave : public Event {};

    struct MouseEvent : public Event {
        U32 buttons;
        MouseEvent(S32 globalX, S32 globalY, U32 buttons) : buttons{buttons} {
            bubble = Bubble::Up;
            this->globalX = globalX;
            this->globalY = globalY;
        }
        virtual Vector<String> toStrings(const String& name) const {
            auto ret = Event::toStrings(name);
            ret.push_back(std::to_string(buttons));
            return ret;
        }
    };

    struct MouseMove : public MouseEvent {
        MouseMove(S32 globalX, S32 globalY, U32 buttons) : MouseEvent{globalX, globalY, buttons} {}
    };

    struct MouseDown : public MouseEvent {
        MouseDown(S32 globalX, S32 globalY, U32 buttons) : MouseEvent{globalX, globalY, buttons} {}
    };

    struct MouseUp : public MouseEvent {
        MouseUp(S32 globalX, S32 globalY, U32 buttons) : MouseEvent{globalX, globalY, buttons} {}
    };

    struct Click : public MouseEvent {
        Click(S32 globalX, S32 globalY, U32 buttons) : MouseEvent{globalX, globalY, buttons} {}
    };

    struct KeyEvent : public Event {
        U32 scancode, keycode;
        const char* keyname;
        KeyEvent(U32 scancode, U32 keycode, const char* keyname) :
            scancode{scancode},
            keycode{keycode},
            keyname{keyname} {
            bubble = Bubble::Up;
        }
    };

    struct KeyDown : public KeyEvent {
        KeyDown(U32 scancode, U32 keycode, const char* keyname) : KeyEvent{scancode, keycode, keyname} {}
    };

    struct KeyUp : public KeyEvent {
        KeyUp(U32 scancode, U32 keycode, const char* keyname) : KeyEvent{scancode, keycode, keyname} {}
    };

    struct Drag : public Event {
        U32 buttons;
        S32 x, y;
        S32 initialX, initialY;
        S32 anchorX, anchorY;
    };

    struct Drop : public Event {
        Drop(S32 x, S32 y) {
            globalX = x;
            globalY = y;
        }
    };
}
