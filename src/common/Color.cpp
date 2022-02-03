// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Color.hpp>

void Color::fromString(const String& color) {
    if (color.empty())
        return;
    if (color[0] == '#') {
        U32 rgba = 0xFF;
        std::from_chars(color.c_str() + 1, color.c_str() + color.size(), rgba, 16);
        if (color.size() == 7) {
            rgba |= 0xFF000000;
        } else if (color.size() == 4) {
            rgba = 0xFF000000 |
                ((rgba & 0xF00) << 24) | ((rgba & 0xF00) << 16) |
                ((rgba & 0x0F0) << 8) | ((rgba & 0x0F0) << 4) |
                ((rgba & 0x00F) << 4) | (rgba & 0x00F);
        }
        fromABGR(rgba);
        return;
    }
    if (color.back() == '}') {
        auto parts = split(color, "{");
        if (parts.size() != 2)
            return;
        auto keys = trim(tolower(parts[0]));
        auto values = split(parts[1], ",");
        for (U32 i = 0; i < keys.size() && i < values.size(); ++i) {
            auto key = keys[i];
            auto value = std::stoi(trim(values[i]));
            if (key == 'r') r = value;
            else if (key == 'g') g = value;
            else if (key == 'b') b = value;
            else if (key == 'a') a = value;
            else continue;
        }
        return;
    }

    fromU32(std::atol(color.c_str()));
}
