// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>

class Undo : public Command {
public:
    void undo() override {}
    void run() override {
        logV("Undo");
        if (auto doc = this->doc())
            doc->undo();
    }
};

static Command::Shared<Undo> undo{"undo"};
