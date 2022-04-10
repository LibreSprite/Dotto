// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <regex>

#include <common/String.hpp>
#include <common/types.hpp>

struct Rect {
    Rect() = default;
    Rect(const Rect&) = default;
    Rect(Rect&&) = default;
    Rect(const String& str) {*this = str;}
    Rect(S32 x, S32 y, U32 w, U32 h) : x{x}, y{y}, width{w}, height{h} {}

    Rect& operator = (Rect& other) = default;
    Rect& operator = (Rect&& other) = default;
    Rect& operator = (const Rect& other) = default;

    bool operator == (const Rect& other) {
        return x == other.x &&
            y == other.y &&
            width == other.width &&
            height == other.height;
    }

    Rect& operator = (const String& str) {
        static std::regex expr("\\s+");
        auto parts = split(str, expr);
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

    Rect& intersect(const Rect& other) {
        S32 x2 = x + width, y2 = y + height;
        S32 ox2 = other.x + other.width, oy2 = other.y + other.height;
        x = std::max(x, other.x);
        y = std::max(y, other.y);
        x2 = std::min(x2, ox2);
        y2 = std::min(y2, oy2);
        width = x2 > x ? x2 - x : 0;
        height = y2 > y ? y2 - y : 0;
        return *this;
    }

    bool empty() const {
        return width == 0 || height == 0;
    }

    bool overlaps(const Rect& other) const {
        bool xoverlaps = !((other.x >= S32(x + width)) || (S32(other.x + other.width) <= x));
        bool yoverlaps = !((other.y >= S32(y + height)) || (S32(other.y + other.height) <= y));
        return xoverlaps && yoverlaps;
    }

    bool contains(S32 px, S32 py) const {
        return px >= x &&
            px < static_cast<S32>(x + width) &&
            py >= y &&
            py < static_cast<S32>(y + height);
    }

    bool expand(const Rect& other) {
        if (other.empty())
            return false;
        bool ret = expand(other.x, other.y);
        bool ret2 = expand(other.right(), other.bottom());
        return ret || ret2;
    }

    void clear() {
        width = height = 0;
    }

    bool empty() {
        return width == 0 || height == 0;
    }

    bool expand(S32 px, S32 py) {
        bool changed = false;
        if (width == 0) {
            x = px;
            y = py;
            width = 1;
            height = 1;
            changed = true;
        } else {
            if (x > px) {
                width += x - px;
                x = px;
                changed = true;
            }
            if (S32(x + width) <= px) {
                width = (px - x) + 1;
                changed = true;
            }
            if (y > py) {
                height += y - py;
                y = py;
                changed = true;
            }
            if (S32(y + height) <= py) {
                height = (py - y) + 1;
                changed = true;
            }
        }
        return changed;
    }

    S32 right() const {return x + width;}
    S32 bottom() const {return y + height;}
    S32 top() const {return y;}
    S32 left() const {return x;}

    S32 x = 0, y = 0;
    U32 width = 0, height = 0;
};
