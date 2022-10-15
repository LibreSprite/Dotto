// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>
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
    PubSub<msg::ModifyCell,
           msg::ActivateCell,
           msg::ActivateLayer> pub{this};
    Property<std::shared_ptr<Cell>> cell{this, "cell", nullptr, &CellPropertyModifier::setCell};
    Property<String> property{this, "property", "", &CellPropertyModifier::setOptions};
    Property<Value> value{this, "value"};
    Property<String> key{this, "key", "value", &CellPropertyModifier::setCell};

    Cell* currentCell = nullptr;

    void setOptions() {
        if (*property == "blendmode") {
            auto& registry = Blender::getRegistry();
            Vector<String> options;
            for (auto& opt : registry)
                options.push_back(opt.first);
            node()->load({{"options", join(options, ",")}});
        }
    }

    std::shared_ptr<Cell> getCell() {
        return *this->cell ?: inject<Cell>{"activecell"}.shared();
    }

    void setCell() {
        auto cell = getCell();
        currentCell = cell.get();

        if (property->empty() || !cell) {
            node()->set("visible", false);
            return;
        }

        node()->set("visible", true);

        if (*property == "alpha")
            node()->set(*key, tostring(cell->getAlpha()));
        else if (*property == "blendmode")
            node()->set(*key, cell->getBlendMode());
        else if (*property == "name")
            node()->set(*key, cell->getName());
    }

    void attach() override {
        node()->addEventListener<ui::Changed>(this);
    }

    void on(msg::ActivateCell&) {
        if (!*cell)
            setCell();
    }

    void on(msg::ActivateLayer&) {
        if (!*cell)
            setCell();
    }

    void on(msg::ModifyCell& event) {
        if (event.cell.get() == currentCell)
            setCell();
    }

    void eventHandler(const ui::Changed&) {
        auto cell = getCell();
        if (!cell)
            return;

        if (auto cmd = inject<Command>{"setcellproperty"}) {
            cmd->load({
                    {"cell", cell},
                    {"property", *property},
                    {"value", *value}
                });
            cmd->run();
        }
    }
};

static ui::Controller::Shared<CellPropertyModifier> reg{"cellpropertymodifier"};
