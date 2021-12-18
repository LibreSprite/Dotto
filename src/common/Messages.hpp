// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <Value.hpp>

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

    struct MouseMove {
        const U32 windowId;
        const S32 x, y;
    };

    struct MouseUp {
        const U32 windowId;
        S32 x, y;
        U32 button;
    };

    struct MouseDown {
        const U32 windowId;
        S32 x, y;
        U32 button;
    };
}
