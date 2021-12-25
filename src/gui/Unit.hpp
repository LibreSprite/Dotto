// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>
#include <regex>

namespace ui {
    class Unit {
    public:
        enum class Type {
            Default,
            Percent,
            Pixel
        };

        constexpr Unit() = default;

        constexpr Unit(const Unit& other) = default;

        constexpr Unit(Unit&& other) = default;

        Unit(const String& str) {*this = str;}

        constexpr Unit(S32 pixel) {*this = pixel;}

        constexpr Unit& operator = (S32 pixel) {
            setPixel(pixel);
            return *this;
        }

        constexpr Unit& operator = (const Unit& other) {
            value = other.value;
            type = other.type;
            reference = other.reference;
            referenceType = other.referenceType;
            return *this;
        }

        constexpr Unit& operator = (Unit&& other) {
            value = other.value;
            type = other.type;
            reference = other.reference;
            referenceType = other.referenceType;
            return *this;
        }

        Unit& operator = (const String& str) {
            auto load = +[](const String& str, F32& value, Type& type){
                if (str.empty()) {
                    type = Type::Default;
                } else {
                    value = std::strtof(str.c_str(), nullptr);
                    auto back = str.back();
                    if (back == '%') {
                        value /= 100.0f;
                        type = Type::Percent;
                    } else {
                        type = Type::Pixel;
                    }
                }
            };
            std::cmatch match;
            std::regex_match(str.c_str(), match, std::regex("(?:([0-9]*(?:px|%?))(\\s*[+-]\\s*))?([0-9]+(?:px|%?))"));

            if (match.empty()) {
                type = Type::Default;
                referenceType = Type::Default;
            } else {
                load(match[1].matched ? match[1].str() : "", reference, referenceType);
                load(match[3].matched ? match[3].str() : "", value, type);
                if (match[2].str() == "-")
                    value = -value;
            }

            return *this;
        }

        bool operator == (const Unit& other) const {
            return other.type == type && other.value == value;
        }

        operator String () const {
            switch (type) {
            case Type::Default: return "";
            case Type::Percent: return std::to_string(value * 100) + "%";
            case Type::Pixel: return std::to_string(static_cast<S32>(value + 0.5f)) + "px";
            }
            return "0";
        }

        constexpr void setPixel(S32 pixel) {
            type = Type::Pixel;
            value = pixel;
        }

        S32 toPixel(S32 parent) const {
            switch (type) {
            case Type::Default:
                return 0;

            case Type::Percent:
                switch (referenceType) {
                case Type::Default:
                    return static_cast<S32>(parent * value + 0.5f);
                case Type::Pixel:
                    return static_cast<S32>(reference + 0.5f) + static_cast<S32>(parent * value + 0.5f);
                case Type::Percent:
                    return static_cast<S32>(parent * reference + 0.5f) + static_cast<S32>(parent * value + 0.5f);
                }
                return 0;

            case Type::Pixel:
                switch (referenceType) {
                case Type::Default:
                    return static_cast<S32>(value + 0.5f);
                case Type::Pixel:
                    return static_cast<S32>(reference + 0.5f) + static_cast<S32>(value + 0.5f);
                case Type::Percent:
                    return static_cast<S32>(parent * reference + 0.5f) + static_cast<S32>(value + 0.5f);
                }
                return 0;
            }
            return 0;
        }

        Type getType() const {
            return type;
        }

    private:
        F32 value = 1.0f;
        Type type = Type::Percent;
        F32 reference = 0;
        Type referenceType = Type::Default;
    };
}
