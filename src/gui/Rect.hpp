// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <regex>

#include <common/String.hpp>
#include <common/types.hpp>

namespace ui {
    struct Rect {
        Rect() = default;
        Rect(const Rect&) = default;
        Rect(Rect&&) = default;
        Rect(const String& str) {*this = str;}

        Rect& operator = (const String& str) {
            auto parts = split(str, std::regex("\\s+"));
            x = parts.size() > 0 ? std::stol(parts[0]) : 0;
            y = parts.size() > 1 ? std::stol(parts[1]) : 0;
            width = parts.size() > 2 ? std::stol(parts[2]) : 0;
            height = parts.size() > 3 ? std::stol(parts[3]) : 0;
            return *this;
        }

        operator String () {
            return std::to_string(x) + " " +
                   std::to_string(y) + " " +
                   std::to_string(width) + " " +
                   std::to_string(height);
        }

        S32 right() {return x + width;}
        S32 bottom() {return y + height;}
        S32 top() {return y;}
        S32 left() {return x;}

        S32 x = 0, y = 0;
        U32 width = 0, height = 0;
    };
}
