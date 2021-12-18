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
}
