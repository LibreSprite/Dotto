// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <any>

#include <common/types.hpp>

class Value {
    std::any value;
    bool (*equal)(const std::any& left, const std::any& right);
    std::shared_ptr<void> (*shared)(const std::any&);
    String (*str)(const std::any&);

public:
    Value() : equal{nullptr} {}

    template <typename Type>
    Value(const Type& v) {*this = v;}

    Value(const Value& v) = default;

    Value(Value&& v) = default;

    Value(const char* v) {*this = String{v};}

    Value(char* v) {*this = String{v};}

    bool empty() const {
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

    std::shared_ptr<void> getShared() const {
        return shared ? shared(value) : nullptr;
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
        if constexpr (is_shared_ptr<Type>::value) {
            shared = +[](const std::any& value) {
                return std::static_pointer_cast<void>(std::any_cast<Type>(value));
            };
        } else {
            shared = nullptr;
        }

        if constexpr (std::is_same_v<Type, String>) {
            str = +[](const std::any& value) {
                return std::any_cast<String>(value);
            };
        } else if constexpr (std::is_convertible_v<Type, String>) {
            str = +[](const std::any& value) {
                return String(std::any_cast<Type>(value));
            };
        } else if constexpr (std::is_same_v<Type, bool> ||
                             std::is_same_v<Type, F32> ||
                             std::is_same_v<Type, F64> ||
                             std::is_same_v<Type, U32> ||
                             std::is_same_v<Type, S32> ||
                             std::is_same_v<Type, U64> ||
                             std::is_same_v<Type, S64>) {
            str = +[](const std::any& value) {
                return std::to_string(std::any_cast<Type>(value));
            };
        } else if constexpr (is_shared_ptr<Type>::value) {
            str = +[](const std::any& value) {
                return "[" + String(typeid(Type).name()) + " " + std::to_string((uintptr_t)std::any_cast<Type>(value).get()) + "]";
            };
        } else {
            str = +[](const std::any& value) {
                return "[" + String(typeid(Type).name()) + "]";
            };
        }
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

    String toString() const {
        return value.has_value() ? (str ? str(value) : String("[not convertible to String: ") + typeName() + "]") : "null";
    }

    const char* typeName() const {
        return value.type().name();
    }
};
