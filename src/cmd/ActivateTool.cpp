// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class ActivateTool : public Command {
    Property<String> tool{this, "tool"};
    PubSub<> pub{this};

public:
    void run() override {
        auto it = Tool::instances.find(tolower(tool));
        if (it == Tool::instances.end()) {
            logE("Invalid tool \"", *tool, "\"");
            return;
        }

        if (Tool::active.lock() == it->second) {
            logV("Tool already active");
            return;
        }

        Tool::previous = Tool::active;
        Tool::active = it->second;
        it->second->onActivate();
        pub(msg::ActivateTool{*tool});
    }
};

static Command::Shared<ActivateTool> cmd{"activatetool"};
