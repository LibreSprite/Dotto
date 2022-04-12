// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class SelectNone : public Command {
    PubSub<> pub{this};

public:
    void run() override {
        if (auto cell = this->cell()) {
            cell->setSelection(nullptr);
        }
    }
};

static Command::Shared<SelectNone> cmd{"selectnone"};
