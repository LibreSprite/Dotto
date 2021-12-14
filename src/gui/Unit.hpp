// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <charconv>
#include <common/types.hpp>
#include <cstdlib>
#include <string>

namespace ui {
    class Unit {
    public:
        enum class Type {
            Percent,
            Pixel
        };

        constexpr Unit() = default;

        constexpr Unit(const Unit& other) : value{other.value}, type{other.type} {}

        constexpr Unit(Unit&& other) : value{other.value}, type{other.type} {}

        Unit(const String& str) {*this = str;}

        constexpr Unit& operator = (const Unit& other) {
            value = other.value;
            type = other.type;
            return *this;
        }

        constexpr Unit& operator = (Unit&& other) {
            value = other.value;
            type = other.type;
            return *this;
        }

        Unit& operator = (const String& str) {
            if (!str.empty()) {
                value = std::strtof(str.c_str(), nullptr);
                auto back = str.back();
                if (back == '%') {
                    type = Type::Percent;
                } else {
                    type = Type::Pixel;
                }
            }
            return *this;
        }

        operator String () {
            switch (type) {
            case Type::Percent: return std::to_string(value * 100) + "%";
            case Type::Pixel: return std::to_string(static_cast<S32>(value + 0.5f)) + "px";
            }
            return "0";
        }

        S32 toPixel(S32 parent) {
            switch (type) {
            case Type::Percent: return static_cast<S32>(parent * value + 0.5f);
            case Type::Pixel: return static_cast<S32>(value + 0.5f);
            }
            return 0;
        }

    private:
        F32 value = 1.0f;
        Type type = Type::Percent;
    };
}
