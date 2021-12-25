// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/types.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

namespace ui {

    S32 Event::targetX() const {
        return target ? globalX - target->globalRect.x : globalX;
    }

    S32 Event::targetY() const {
        return target ? globalY - target->globalRect.y : globalY;
    }

    Vector<String> Event::toStrings(const String& name) const {
        return {
            name,
            target ? *target->id : "",
            std::to_string(targetX()),
            std::to_string(targetY())
        };
    }

}
