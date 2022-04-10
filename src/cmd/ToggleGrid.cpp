// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/PubSub.hpp>
#include <gui/Node.hpp>

class ToggleGrid : public Command {
public:
    void run() override {
        inject<ui::Node> editor{"activeeditor"};
        if (editor) {
            editor->load({{"grid", !editor->getPropertySet().get<bool>("grid")}});
        }
    }
};

static Command::Shared<ToggleGrid> cmd{"togglegrid"};
