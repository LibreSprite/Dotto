// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <variant>

namespace match {
    namespace detail {
        template<typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
        template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
    }

    template<typename V, typename... Ts>
    void variant(V& v, Ts&& ... o) {
        std::visit(detail::overloaded{std::move(o)...}, v);
    }
}
