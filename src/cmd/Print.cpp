// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <log/Log.hpp>

class Print : public Command {
    Property<String> msg{this, "msg"};
public:
    void run() override {
        logI("Print: ", msg);
    }
};

static Command::Shared<Print> print{"print"};
