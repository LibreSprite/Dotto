// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <any>

#include <common/types.hpp>

class Value {
    std::any value;
    bool (*equal)(const std::any& left, const std::any& right);

public:
    Value() : equal{nullptr} {}

    template <typename Type>
    Value(const Type& v) {*this = v;}

    Value(const Value& v) = default;

    Value(Value&& v) = default;

    Value(const char* v) {*this = String{v};}

    Value(char* v) {*this = String{v};}

    bool empty() {
        return !value.has_value() || has<std::nullptr_t>();
    }

    template<typename Type>
    bool has() const {
        return value.has_value() && value.type() == typeid(Type);
    }

    template<typename Type>
    operator Type () const {
        return has<Type>() ? std::any_cast<Type>(value) : Type{};
    }

    template<typename Type>
    Type get() const {
        return std::any_cast<Type>(value);
    }

    Value& operator = (Value&& other) = default;
    Value& operator = (Value& other) = default;
    Value& operator = (const Value& other) = default;

    template<typename Type>
    Value& operator = (const Type& v) {
        value = v;
        equal = +[](const std::any& left, const std::any& right) {
            return std::any_cast<Type>(left) == std::any_cast<Type>(right);
        };
        return *this;
    }

    bool operator == (const Value& other) const {
        if (other.value.has_value() != value.has_value())
            return false;
        if (!value.has_value())
            return true;
        if (value.type() != other.value.type())
            return false;
        return equal(value, other.value);
    }

    const char* typeName() const {
        return value.type().name();
    }
};
