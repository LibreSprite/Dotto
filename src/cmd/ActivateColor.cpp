// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Color.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class ActivateColor : public Command {
    Property<Color> color{this, "color"};
    PubSub<> pub{this};

public:
    void undo() override {}

    void run() override {
        Tool::color = color;
        pub(msg::ActivateColor{*color});
    }
};

static Command::Shared<ActivateColor> cmd{"activatecolor"};
