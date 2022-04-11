// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <gui/Node.hpp>

class NewFile : public Command {
public:
    Property<bool> interactive{this, "interactive", false};
    Property<U32> width{this, "width", 32};
    Property<U32> height{this, "height", 32};

    void run() override {
        if (interactive) {
            ui::Node::fromXML("newfilepopup");
            return;
        }
        if (auto editor = ui::Node::fromXML("editor")) {
            editor->set("newfile", std::make_shared<PropertySet>(getPropertySet()));
        }
    }
};

static Command::Shared<NewFile> newfile{"newfile"};
