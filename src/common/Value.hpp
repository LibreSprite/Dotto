// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <any>

class Value {
    std::any value;

public:
    template <typename Type>
    Value(const Type& v) : value(v) {}

    Value(const Value& v) : value(v.value) {}

    Value(Value&& v) : value(std::move(v.value)) {}

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

    template<typename Type>
    Value& operator = (const Type& v) {
        value = v;
        return *this;
    }

    const char* typeName() const {
        return value.type().name();
    }
};
