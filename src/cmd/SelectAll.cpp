// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class SelectAll : public Command {
    PubSub<> pub{this};

public:
    void run() override {
        if (auto cell = this->cell()) {
            inject<Selection> selection{"new"};
            selection->add({0, 0, doc()->width(), doc()->height()}, 255);
            cell->setSelection(selection);
        }
    }
};

static Command::Shared<SelectAll> cmd{"selectall"};
