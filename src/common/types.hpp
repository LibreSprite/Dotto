// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using U64 = std::uint64_t;
using S64 = std::int64_t;
using U32 = std::uint32_t;
using S32 = std::int32_t;
using U16 = std::uint16_t;
using S16 = std::int16_t;
using U8 = std::uint8_t;
using S8 = std::int8_t;
using F32 = float;
using F64 = double;

using String = std::string;

template<typename Type>
using Vector = std::vector<Type>;

template<typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value>;

template<typename Key>
using HashSet = std::unordered_set<Key>;

struct Point2D {
    S32 x;
    S32 y;
};


template<class T>
struct is_shared_ptr : std::false_type {};

template<class T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template<typename Func>
class FunctionRef {
    std::shared_ptr<std::function<Func>> func;

public:
    FunctionRef() = default;

    FunctionRef(const FunctionRef& func) : func{func.func} {}

    FunctionRef(const std::function<Func>& func) : func{std::make_shared<std::function<Func>>(func)} {}

    FunctionRef(std::function<Func>&& func) : func{std::make_shared<std::function<Func>>(std::move(func))} {}

    operator bool () {
        return func != nullptr && *func;
    }

    bool operator == (const FunctionRef<Func>& other) {
        return func == other.func;
    }

    template<typename ... Args>
    auto operator () (Args&& ... args) {
        return (*func)(std::forward<Args>(args)...);
    }
};
