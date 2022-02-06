// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <gui/Node.hpp>

class Quit : public Command {
    PubSub<> pub{this};
public:
    void run() override {
        pub(msg::RequestShutdown{});
    }
};

static Command::Shared<Quit> quit{"quit"};
