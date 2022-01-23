// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "log/Log.hpp"
#include <cmd/Command.hpp>

class Redo : public Command {
public:
    void run() override {
        if (auto doc = this->doc())
            doc->redo();
    }
};

static Command::Shared<Redo> redo{"redo"};
