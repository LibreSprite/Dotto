// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <any>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <functional>

#include <common/types.hpp>
#include <log/Log.hpp>

class Value {
    struct converterHashFunc {
        std::size_t operator () (const std::pair<std::type_index, std::type_index>& pair) const {
            return pair.first.hash_code() * 31 + pair.second.hash_code();
        }
    };

    using Converter = std::function<void(const std::any&, void*)>;

    using ConverterMap = std::unordered_map<std::pair<std::type_index, std::type_index>, Converter, converterHashFunc>;

    static ConverterMap& getConverters() {
        static ConverterMap converters;
        return converters;
    }

    template <typename T>
    struct arg_type : public arg_type<decltype(&T::operator())> {};

    template <typename ReturnType, typename ... Args>
    struct arg_type<ReturnType(&)(Args...)> {
        using type = std::remove_reference_t<decltype(std::get<0>(std::tuple<std::remove_reference_t<std::remove_cv_t<Args>>...>()))>;
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct arg_type<ReturnType(ClassType::*)(Args...) const> {
        using type = std::remove_reference_t<decltype(std::get<0>(std::tuple<std::remove_reference_t<std::remove_cv_t<Args>>...>()))>;
    };

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

    template<typename Type, bool exact = false>
    bool has() const {
        if (!value.has_value())
            return false;

        if constexpr (std::is_same_v<Type, Value>)
            return true;

        auto key = std::make_pair(std::type_index(value.type()), std::type_index(typeid(Type)));
        if (key.first == key.second)
            return true;

        if (!exact) {
            auto& converters = getConverters();
            auto it = converters.find(key);
            return it != converters.end();
        }

        return false;
    }

    template<typename Type, bool exact = false>
    bool get(Type& out) const {
        if (!value.has_value())
            return false;

        if constexpr (std::is_same_v<Type, Value>) {
            out = *this;
            return true;
        }

        auto key = std::make_pair(std::type_index(value.type()), std::type_index(typeid(Type)));
        if (key.first == key.second) {
            out = std::any_cast<Type>(value);
            return true;
        }

        if (exact) {
            return false;
        }

        auto& converters = getConverters();
        auto it = converters.find(key);
        if (it == converters.end())
            return false;

        it->second(value, &out);
        return true;
    }

    template<typename Type>
    operator Type () const {
        auto key = std::make_pair(std::type_index(value.type()), std::type_index(typeid(Type)));

        if (key.first == key.second)
            return std::any_cast<Type>(value);

        auto& converters = getConverters();
        auto it = converters.find(key);

        Type ret{};
        if (it != converters.end())
            it->second(value, &ret);
        else {
            logE("Could not create ", typeid(Type).name(), " out of ", value.type().name());
        }
        return ret;
    }

    template<typename Type>
    Type get() const {
        return static_cast<Type>(*this);
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

    template<typename Func, typename This = typename arg_type<Func>::type, typename That = std::result_of_t<Func(This)>>
    static void addConverter(const Func& func) {
        auto& converters = getConverters();
        auto key = std::make_pair(std::type_index(typeid(This)), std::type_index(typeid(That)));
        converters[key] = [func](const std::any& value, void* target) {
            *reinterpret_cast<That*>(target) = func(std::any_cast<This>(value));
        };
    }

    template<typename Type>
    static void addIntConverters() {
        addConverter([](Type v){return Value{v};});
        addConverter([](Type v){return static_cast<float>(v);});
        addConverter([](Type v){return static_cast<signed char>(v);});
        addConverter([](Type v){return static_cast<bool>(v);});
        addConverter([](Type v){return static_cast<char>(v);});
        addConverter([](Type v){return static_cast<short>(v);});
        addConverter([](Type v){return static_cast<int>(v);});
        addConverter([](Type v){return static_cast<long>(v);});
        addConverter([](Type v){return static_cast<long long>(v);});
        addConverter([](Type v){return static_cast<double>(v);});
        addConverter([](Type v){return static_cast<unsigned char>(v);});
        addConverter([](Type v){return static_cast<unsigned short>(v);});
        addConverter([](Type v){return static_cast<unsigned int>(v);});
        addConverter([](Type v){return static_cast<unsigned long>(v);});
        addConverter([](Type v){return static_cast<unsigned long long>(v);});
        addConverter([](Type v){return std::to_string(v);});
        addConverter([](const std::string& str){return static_cast<Type>(atoi(str.c_str()));});
        addConverter([](const char* str){return static_cast<Type>(atoi(str));});
        addConverter([](char* str){return static_cast<Type>(atoi(str));});
    }

    template<typename Type>
    static void addFloatConverters() {
        addConverter([](Type v){return Value{v};});
        addConverter([](Type v){return static_cast<float>(v);});
        addConverter([](Type v){return static_cast<double>(v);});
        addConverter([](Type v){return v != 0;});
        addConverter([](Type v){return static_cast<signed char>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<char>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<short>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<int>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<long>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<long long>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<unsigned char>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<unsigned short>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<unsigned int>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<unsigned long>(v + Type(0.5));});
        addConverter([](Type v){return static_cast<unsigned long long>(v + Type(0.5));});
        addConverter([](Type v){return std::to_string(v);});
        addConverter([](const std::string& str){return static_cast<Type>(atof(str.c_str()));});
        addConverter([](const char* str){return static_cast<Type>(atof(str));});
        addConverter([](char* str){return static_cast<Type>(atof(str));});
    }

    static void addBasicConverters() {
        addIntConverters<signed char>();
        addIntConverters<char>();
        addIntConverters<short>();
        addIntConverters<int>();
        addIntConverters<long>();
        addIntConverters<long long>();
        addIntConverters<double>();
        addIntConverters<unsigned char>();
        addIntConverters<unsigned short>();
        addIntConverters<unsigned int>();
        addIntConverters<unsigned long>();
        addIntConverters<unsigned long long>();
        addFloatConverters<float>();
        addFloatConverters<double>();

        addConverter([](const std::string& v){return Value{v};});

        addConverter([](bool v){return v ? "true" : "false";});
        addConverter([](bool v) -> std::string {return v ? "true" : "false";});
        addConverter([](const char* v){return v && v[0] == 't';});
        addConverter([](char* v){return v && v[0] == 't';});
        addConverter([](const std::string& v){return v == "true";});
        addConverter([](bool v){return Value{v};});
    }
};
