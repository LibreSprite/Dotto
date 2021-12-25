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

    struct MouseMove : public Event {
        U32 buttons;
        MouseMove(S32 globalX, S32 globalY, U32 buttons) : buttons{buttons} {
            bubble = Bubble::Up;
            this->globalX = globalX;
            this->globalY = globalY;
        }
    };

    struct MouseDown : public Event {
        U32 buttons;
        MouseDown(S32 globalX, S32 globalY, U32 buttons) : buttons{buttons} {
            bubble = Bubble::Up;
            this->globalX = globalX;
            this->globalY = globalY;
        }
    };

    struct MouseUp : public Event {
        U32 buttons;
        MouseUp(S32 globalX, S32 globalY, U32 buttons) : buttons{buttons} {
            bubble = Bubble::Up;
            this->globalX = globalX;
            this->globalY = globalY;
        }
    };

    struct Click : public Event {
        U32 buttons;
        Click(S32 globalX, S32 globalY, U32 buttons) : buttons{buttons} {
            bubble = Bubble::Up;
            this->globalX = globalX;
            this->globalY = globalY;
        }
    };

    struct KeyEvent : public Event {
        U32 scancode, keycode;
        KeyEvent(U32 scancode, U32 keycode) : scancode{scancode}, keycode{keycode} {
            bubble = Bubble::Up;
        }
    };

    struct KeyDown : public KeyEvent {
        KeyDown(U32 scancode, U32 keycode) : KeyEvent{scancode, keycode} {}
    };

    struct KeyUp : public KeyEvent {
        KeyUp(U32 scancode, U32 keycode) : KeyEvent{scancode, keycode} {}
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
