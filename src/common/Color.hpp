// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <charconv>
#include <common/types.hpp>
#include <common/String.hpp>

class Color {
public:
    U8 r = 0, g = 0, b = 0, a = 255;
    static inline constexpr const U32 Rshift = 0;
    static inline constexpr const U32 Gshift = 8;
    static inline constexpr const U32 Bshift = 16;
    static inline constexpr const U32 Ashift = 24;

    constexpr Color() = default;

    constexpr Color(const Color& other) = default;

    constexpr Color(U32 rgba) {
        fromU32(rgba);
    }

    Color(const String& color) {
        fromString(color);
    }

    constexpr Color(U8 r, U8 g, U8 b, U8 a = 255) : r{r}, g{g}, b{b}, a{a} {}

    operator String () const {
        return "rgba{" + std::to_string(r) +
            "," + std::to_string(g) +
            "," + std::to_string(b) +
            "," + std::to_string(a) +
            "}";
    }

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

    constexpr Color& fromU32(U32 rgba) {
        r = static_cast<U8>(rgba >> Rshift);
        g = static_cast<U8>(rgba >> Gshift);
        b = static_cast<U8>(rgba >> Bshift);
        a = static_cast<U8>(rgba >> Ashift);
        return *this;
    }

    constexpr U32 toU32() const {
        return (U32{a} << Ashift) |
            (U32{b} << Bshift) |
            (U32{g} << Gshift) |
            (U32{r} << Rshift);
    }

    constexpr U32 distanceSquared(const Color& other) const {
        S32 dr = S32{r} - S32{other.r};
        S32 dg = S32{g} - S32{other.g};
        S32 db = S32{b} - S32{other.b};
        S32 da = S32{a} - S32{other.a};
        return dr*dr + dg*dg + db*db + da*da;
    }

    constexpr bool operator == (const Color& other) const {
        return r == other.r &&
            g == other.g &&
            b == other.b &&
            a == other.a;
    }

    constexpr bool operator != (const Color& other) const {
        return !(*this == other);
    }
};
