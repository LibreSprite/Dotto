// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <charconv>
#include <common/types.hpp>
#include <common/String.hpp>
#include <log/Log.hpp>

class Color {
public:
    U8 r = 0, g = 0, b = 0, a = 255;

    Color() = default;

    Color(const Color& other) = default;

    Color(U32 rgba) {
        fromU32(rgba);
    }

    Color(const String& color) {
        fromString(color);
    }

    Color(U8 r, U8 g, U8 b, U8 a = 255) : r{r}, g{g}, b{b}, a{a} {}

    void fromString(const String& color) {
        if (color.empty())
            return;
        if (color[0] == '#') {
            U32 rgba = 0xFF;
            std::from_chars(color.c_str() + 1, color.c_str() + color.size(), rgba, 16);
            fromU32(rgba);
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
    }

    Color& fromU32(U32 rgba) {
        r = static_cast<U8>(rgba >> 24);
        g = static_cast<U8>(rgba >> 16);
        b = static_cast<U8>(rgba >> 8);
        a = static_cast<U8>(rgba);
        return *this;
    }
};
