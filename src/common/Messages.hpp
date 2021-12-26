// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Value.hpp>

namespace ui {
    class Node;
}

namespace msg {

    class BootComplete{};
    class Shutdown{};

    class Flush{
        const Value& resource;
        bool held = false;
    public:
        Flush(const Value& resource) : resource{resource} {};

        void hold(const Value& other) {
            if (!held)
                held = other == resource;
        }

        bool isHeld() {
            return held;
        }
    };

    struct WindowMaximized {const U32 windowId;};
    struct WindowMinimized {const U32 windowId;};
    struct WindowRestored {const U32 windowId;};
    struct WindowClosed {const U32 windowId;};

    struct MouseMove {
        const U32 windowId;
        const S32 x, y;
        U32 buttons;
    };

    struct MouseUp {
        const U32 windowId;
        S32 x, y;
        U32 buttons;
    };

    struct MouseDown {
        const U32 windowId;
        S32 x, y;
        U32 buttons;
    };

    struct KeyUp {
        const U32 windowId;
        U32 scancode;
        const char* keyName;
        U32 keycode;
    };

    struct KeyDown {
        const U32 windowId;
        U32 scancode;
        const char* keyName;
        U32 keycode;
    };

    struct BeginDrag {
        std::shared_ptr<ui::Node> target;
        S32 anchorX, anchorY;
    };

    struct EndDrag {
        std::shared_ptr<ui::Node> target;
    };

    struct ActivateTool {
        const String& tool;
    };
}
