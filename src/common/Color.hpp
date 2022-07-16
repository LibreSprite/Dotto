// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <charconv>
#include <common/types.hpp>
#include <common/String.hpp>
#include <common/Value.hpp>

class Color {
public:
    U8 r = 0, g = 0, b = 0, a = 255;

    #ifdef COLOR_R_SHIFT
    static inline constexpr const U32 Rshift = COLOR_R_SHIFT;
    #else
    static inline constexpr const U32 Rshift = 0;
    #endif

    #ifdef COLOR_R_SHIFT
    static inline constexpr const U32 Gshift = COLOR_G_SHIFT;
    #else
    static inline constexpr const U32 Gshift = 8;
    #endif

    #ifdef COLOR_R_SHIFT
    static inline constexpr const U32 Bshift = COLOR_B_SHIFT;
    #else
    static inline constexpr const U32 Bshift = 16;
    #endif

    #ifdef COLOR_R_SHIFT
    static inline constexpr const U32 Ashift = COLOR_A_SHIFT;
    #else
    static inline constexpr const U32 Ashift = 24;
    #endif


    constexpr Color() = default;

    constexpr Color(const Color& other) = default;

    constexpr Color(U32 rgba) {
        fromU32(rgba);
    }

    Color(const String& color) {
        fromString(color);
    }

    constexpr Color(U8 r, U8 g, U8 b, U8 a = 255) : r{r}, g{g}, b{b}, a{a} {}

    static void addConverters() {
        Value::addConverter([](const String& str) -> Color {return str;});
        Value::addConverter([](F32 pixel) -> Color {return static_cast<S32>(pixel);});
        Value::addConverter([](F64 pixel) -> Color {return static_cast<S32>(pixel);});
        Value::addConverter([](U32 pixel) -> Color {return pixel;});
        Value::addConverter([](U64 pixel) -> Color {return static_cast<U32>(pixel);});
        Value::addConverter([](S32 pixel) -> Color {return static_cast<U32>(pixel);});
        Value::addConverter([](S64 pixel) -> Color {return static_cast<U32>(pixel);});
    }

    operator String () const {
        return toString();
    }

    String toString() const {
        return "rgba{" + std::to_string(r) +
            "," + std::to_string(g) +
            "," + std::to_string(b) +
            "," + std::to_string(a) +
            "}";
    }

    void fromString(const String& color);

    constexpr Color& fromU32(U32 rgba) {
        r = static_cast<U8>(rgba >> Rshift);
        g = static_cast<U8>(rgba >> Gshift);
        b = static_cast<U8>(rgba >> Bshift);
        a = static_cast<U8>(rgba >> Ashift);
        return *this;
    }

    constexpr Color& fromABGR(U32 rgba) {
        r = static_cast<U8>(rgba >> 16);
        g = static_cast<U8>(rgba >> 8);
        b = static_cast<U8>(rgba >> 0);
        a = static_cast<U8>(rgba >> 24);
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
