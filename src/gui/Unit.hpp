// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <regex>
#include <string_view>

#include <common/Value.hpp>

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

        Unit(const char* str) {*this = str;}

        constexpr Unit(S32 pixel) {*this = pixel;}

        Unit(const Value& pixel) {
            if (pixel.has<U32>())
                *this = static_cast<U32>(pixel);
            else if (pixel.has<S32>())
                *this = static_cast<S32>(pixel);
            else if (pixel.has<F32>())
                *this = U32(F32(pixel));
            else if (pixel.has<F64>())
                *this = U32(F64(pixel));
            else if (pixel.has<String>())
                *this = pixel.get<String>();
        }

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
            return *this = str.c_str();
        }

        Unit& operator = (char* str) {
            return *this = const_cast<const char*>(str);
        }

        Unit& operator = (const char* str) {
            std::string_view view = str;
            if (view == "center")
                str = "50%-50%";
            else if (view == "top" || view == "left")
                str = "0px";
            else if (view == "bottom" || view == "right")
                str = "100%-100%";

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
            static std::regex expr{"(?:([0-9]*(?:px|%?))(\\s*[+-]\\s*))?([0-9]+(?:px|%?))"};
            std::regex_match(str, match, expr);

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
            return other.type == type && other.value == value &&
                other.referenceType == referenceType && other.reference == reference;
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
            referenceType = Type::Default;
            reference = 0;
        }

        S32 toPixel(S32 parent, S32 own) const {
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
                    return static_cast<S32>(parent * reference + 0.5f) + static_cast<S32>(own * value + 0.5f);
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
