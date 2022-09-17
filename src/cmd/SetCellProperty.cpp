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
    Property<std::shared_ptr<Cell>> targetCell{this, "cell"};
    Property<String> property{this, "property"};
    Property<Value> value{this, "value"};
    Value prevValue;

public:
    void undo() override {
        if (*property == "alpha")
            (*targetCell)->setAlpha(prevValue, false);
        else if (*property == "blendmode")
            (*targetCell)->setBlendMode(prevValue, false);
        else if (*property == "name")
            (*targetCell)->setName(prevValue, false);
    }

    void run() override {
        auto doc = this->doc();
        if (!doc)
            return;
        if (!*targetCell)
            *targetCell = cell();
        if (!*targetCell)
            return;
        if (*property == "alpha") {
            prevValue = (*targetCell)->getAlpha();
            *value = getPropertySet().get<F32>("value");
            (*targetCell)->setAlpha(*value, false);
        } else if (*property == "blendmode") {
            prevValue = (*targetCell)->getBlendMode();
            *value = getPropertySet().get<String>("value");
            (*targetCell)->setBlendMode(*value, false);
        } else if (*property == "name") {
            prevValue = (*targetCell)->getName();
            *value = getPropertySet().get<String>("value");
            (*targetCell)->setName(*value, false);
        } else {
            logE("Invalid SetCellProperty property [", *property, "]");
            return;
        }
        if (prevValue == *value)
            return;
        auto prev = std::dynamic_pointer_cast<SetCellProperty>(doc->getLastCommand());
        if (prev && *prev->targetCell == *targetCell && *prev->property == *property) {
            prev->set("value", *value);
        } else {
            commit();
        }
    }
};

static Command::Shared<SetCellProperty> cmd{"setcellproperty"};
