// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <functional>
#include <regex>
#include <common/PropertySet.hpp>
#include <cmd/Command.hpp>

template<typename Func>
class FunctionRef {
    std::shared_ptr<std::function<Func>> func;

public:
    FunctionRef() = default;

    FunctionRef(const FunctionRef& func) : func{func.func} {}

    FunctionRef(const std::function<Func>& func) : func{std::make_shared<std::function<Func>>(func)} {}

    FunctionRef(std::function<Func>&& func) : func{std::make_shared<std::function<Func>>(std::move(func))} {}

    FunctionRef(const String& commands) : FunctionRef([=]{
        for (auto& command : split(commands, ";")) {
            std::cmatch match;
            std::regex_match(command.c_str(), match, std::regex("^\\s*([^\\s]+)\\s+(.*)$"));
            if (match.empty()) {
                logI("Invalid command [", command, "]");
                return;
            }

            auto name = tolower(match[1]);
            auto args = split(match[2], ",");
            PropertySet ps;
            for (auto& arg : args) {
                auto it = arg.find('=');
                if (it == String::npos)
                    continue;
                auto key = trim(arg.substr(0, it));
                auto value = trim(arg.substr(it + 1));
                ps.set(key, value);
            }

            inject<Command> cmd{name};
            if (!cmd)
                return;
            cmd->load(ps);
            cmd->run();
        }
    }) {}

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
