// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <any>

class Value {
    std::any value;

public:
    template<typename Type>
    bool has() {
        return value.has_value() && value.type() == typeid(Type);
    }

    template<typename Type>
    operator Type () {
        return has<Type>() ? std::any_cast<Type>() : Type{};
    }

    template<typename Type>
    Value& operator = (const Type& v) {
        value = v;
        return *this;
    }
};
