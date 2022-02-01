// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Unit.hpp>

class CellPropertyModifier : public ui::Controller {
public:
    PubSub<> pub{this};
    Property<std::shared_ptr<Cell>> cell{this, "cell", nullptr, &CellPropertyModifier::setCell};
    Property<String> property{this, "property"};

    void setCell() {
        if (property->empty() || !*cell)
            return;
        if (*property == "alpha")
            node()->set("value", tostring((*cell)->getAlpha()));
    }

    void attach() override {
        node()->addEventListener<ui::Changed>(this);
    }

    template<typename Type>
    Type value() {
        return node()->getPropertySet().get<Type>("value");
    }

    void eventHandler(const ui::Changed&) {
        if (property->empty() || !*cell)
            return;
        if (*property == "alpha") {
            (*cell)->setAlpha(value<F32>(), false);
        }
    }
};

static ui::Controller::Shared<CellPropertyModifier> reg{"cellpropertymodifier"};
