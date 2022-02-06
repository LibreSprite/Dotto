// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <tools/Tool.hpp>

class IncreaseToolSize : public Command {
public:
    Property<U32> amount{this, "amount", 1};

    void run() override {
        if (auto tool = Tool::active.lock()) {
            auto& ps = tool->getPropertySet();
            auto size = ps.get<S32>("size");
            tool->set("size", size + amount);
        }
    }
};

static Command::Shared<IncreaseToolSize> cmd{"increasetoolsize"};
