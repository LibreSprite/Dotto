// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class ToggleTool : public Command {
    PubSub<> pub{this};

public:
    void run() override {
        std::swap(Tool::previous, Tool::active);
        if (auto tool = Tool::active.lock()) {
            tool->onActivate();
            if (auto name = tool->get("tool")) {
                pub(msg::ActivateTool{name->get<String>()});
                return;
            }
        }
        pub(msg::ActivateTool{""});
    }
};

static Command::Shared<ToggleTool> cmd{"toggletool"};
