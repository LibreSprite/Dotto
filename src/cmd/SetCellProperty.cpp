// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <doc/Cell.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <memory>

class SetCellProperty : public Command {
    inject<ui::Node> editor{"activeeditor"};
    Property<std::shared_ptr<Cell>> cell{this, "cell"};
    Property<String> property{this, "property"};
    Property<Value> value{this, "value"};
    Value prevValue;

public:
    void undo() override {
        if (*property == "alpha")
            (*cell)->setAlpha(prevValue, false);
    }

    void run() override {
        if (!*cell)
            *cell = inject<Cell>{"activecell"};
        if (!*cell)
            return;
        if (*property == "alpha") {
            prevValue = (*cell)->getAlpha();
            *value = getPropertySet().get<F32>("value");
            (*cell)->setAlpha(*value, false);
        } else if (*property == "blendmode") {
            prevValue = (*cell)->getBlendMode();
            *value = getPropertySet().get<String>("value");
            (*cell)->setBlendMode(*value, false);
        } else {
            logE("Invalid SetCellProperty property [", *property, "]");
            return;
        }
        if (prevValue == *value)
            return;
        auto prev = std::dynamic_pointer_cast<SetCellProperty>(doc()->getLastCommand());
        if (prev && *prev->cell == *cell && *prev->property == *property) {
            prev->set("value", *value);
        } else {
            commit();
        }
    }
};

static Command::Shared<SetCellProperty> cmd{"setcellproperty"};
