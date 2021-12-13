// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace ui {

    class Node;

    class Event {
    public:
        virtual ~Event() = default;
        Node* target = nullptr;
        enum class Bubble {
            None,
            Up,
            Down
        };
        Bubble bubble = Bubble::None;
        bool cascade = true;
        mutable bool cancel = false;
    };

    struct AddToScene : public Event {
        AddToScene() {
            bubble = Bubble::Down;
        }
    };

    struct RemoveFromScene : public Event {
        RemoveFromScene() {
            bubble = Bubble::Down;
        }
    };
}
