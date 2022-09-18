// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Color.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class ToggleIDE : public Command {

public:
    void run() override {
        inject<ui::Node> editor{"activeeditor"};
        if (!editor)
            return;
        auto ide = editor->getPropertySet().get<std::shared_ptr<ui::Node>>("ide");
        if (!ide) {
            ide = ui::Node::fromXML("ide");
            editor->addChild(ide);
            editor->load({{"ide", ide}});
        }
        ide->load({{"visible", !*ide->visible}});
    }
};

static Command::Shared<ToggleIDE> cmd{"toggleide"};
