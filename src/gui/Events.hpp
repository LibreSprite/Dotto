// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>

namespace ui {

    class Node;

    class Event {
    public:
        Event(Node* target) : target{target} {}
        virtual ~Event() = default;

        mutable Node* target = nullptr;
        mutable Node* currentTarget = nullptr;

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
        AddToScene(Node* target) : Event{target} {
            bubble = Bubble::Down;
        }
    };

    struct Changed : public Event {
        Changed(Node* target) : Event{target} {
            bubble = Bubble::Up;
        }
    };

    struct Resize : public Event {
        Resize(Node* target) : Event{target} {}
    };

    struct Remove : public Event {
        Remove(Node* target) : Event{target} {}
    };

    struct Blur : public Event {
        Blur(Node* target) : Event{target} {}
    };

    struct Focus : public Event {
        Focus(Node* target) : Event{target} {}
    };

    struct BlurChild : public Event {
        BlurChild(Node* target) : Event{target} {
            bubble = Bubble::Up;
        }
    };

    struct FocusChild : public Event {
        FocusChild(Node* target) : Event{target} {
            bubble = Bubble::Up;
        }
    };


    struct RemoveFromScene : public Event {
        RemoveFromScene(Node* target) : Event{target} {
            bubble = Bubble::Down;
        }
    };

    struct MouseEnter : public Event {
        MouseEnter(Node* target) : Event{target} {
            bubble = Bubble::Up;
        }
    };

    struct MouseLeave : public Event {
        MouseLeave(Node* target) : Event{target} {
            bubble = Bubble::Up;
        }
    };

    struct MouseEvent : public Event {
        U32 buttons;
        MouseEvent(Node* target, S32 globalX, S32 globalY, U32 buttons) : Event{target}, buttons{buttons} {
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
        MouseMove(Node* target, S32 globalX, S32 globalY, U32 buttons) :
            MouseEvent{target, globalX, globalY, buttons} {}
    };

    struct MouseWheel : public MouseEvent {
        S32 wheelX, wheelY;
        MouseWheel(Node* target, S32 globalX, S32 globalY, U32 buttons, S32 wheelX, S32 wheelY) :
            MouseEvent{target, globalX, globalY, buttons},
            wheelX{wheelX},
            wheelY{wheelY}{}
    };

    struct MouseDown : public MouseEvent {
        MouseDown(Node* target, S32 globalX, S32 globalY, U32 buttons) :
            MouseEvent{target, globalX, globalY, buttons} {}
    };

    struct MouseUp : public MouseEvent {
        MouseUp(Node* target, S32 globalX, S32 globalY, U32 buttons) :
            MouseEvent{target, globalX, globalY, buttons} {}
    };

    struct Click : public MouseEvent {
        Click(Node* target, S32 globalX, S32 globalY, U32 buttons) :
            MouseEvent{target, globalX, globalY, buttons} {}
    };

    struct KeyEvent : public Event {
        U32 scancode, keycode;
        const char* keyname;
        std::unordered_set<String>& pressedKeys;
        KeyEvent(Node* target,
                 U32 scancode,
                 U32 keycode,
                 const char* keyname,
                 std::unordered_set<String>& pressedKeys) :
            Event{target},
            scancode{scancode},
            keycode{keycode},
            keyname{keyname},
            pressedKeys{pressedKeys} {
            bubble = Bubble::Up;
        }
    };

    struct KeyDown : public KeyEvent {
        KeyDown(Node* target,
                U32 scancode,
                U32 keycode,
                const char* keyname,
                std::unordered_set<String>& pressedKeys) :
            KeyEvent{target, scancode, keycode, keyname, pressedKeys} {}
    };

    struct KeyUp : public KeyEvent {
        KeyUp(Node* target,
              U32 scancode,
              U32 keycode,
              const char* keyname,
              std::unordered_set<String>& pressedKeys) :
            KeyEvent{target, scancode, keycode, keyname, pressedKeys} {}
    };

    struct Drag : public Event {
        Drag(Node* target) : Event{target} {}
        U32 buttons;
        S32 x, y;
        S32 initialX, initialY;
        S32 anchorX, anchorY;
    };

    struct Drop : public Event {
        Drop(Node* target, S32 x, S32 y) : Event{target} {
            globalX = x;
            globalY = y;
        }
    };
}
