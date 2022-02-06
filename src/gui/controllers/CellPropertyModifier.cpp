// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Unit.hpp>

class CellPropertyModifier : public ui::Controller {
public:
    PubSub<msg::ModifyCell> pub{this};
    Property<std::shared_ptr<Cell>> cell{this, "cell", nullptr, &CellPropertyModifier::setCell};
    Property<String> property{this, "property"};
    Property<Value> value{this, "value"};

    void setCell() {
        if (property->empty() || !*cell)
            return;
        if (*property == "alpha")
            node()->set("value", tostring((*cell)->getAlpha()));
    }

    void attach() override {
        node()->addEventListener<ui::Changed>(this);
    }

    void on(msg::ModifyCell& event) {
        if (event.cell == *cell)
            setCell();
    }

    void eventHandler(const ui::Changed&) {
        if (!*cell)
            return;
        inject<Command> cmd{"setcellproperty"};
        if (cmd) {
            cmd->load({
                    {"cell", *cell},
                    {"property", *property},
                    {"value", *value}
                });
            cmd->run();
        }
    }
};

static ui::Controller::Shared<CellPropertyModifier> reg{"cellpropertymodifier"};
