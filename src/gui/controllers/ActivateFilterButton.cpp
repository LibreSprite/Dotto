// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <filters/Filter.hpp>

class ActivateFilterButton : public ui::Controller {
public:
    Property<String> filter{this, "filter"};
    Property<bool> interactive{this, "interactive", true};

    void attach() override {
        node()->addEventListener<ui::Click>(this);
    }

    void eventHandler(const ui::Click& event) {
        if (auto command = inject<Command>{"activatefilter"}) {
            if (interactive) {
                command->load(node()->getPropertySet());
            } else {
                PropertySet properties{{"filter", *filter}};
                command->load(properties);
            }
            command->run();
        }
    }
};

static ui::Controller::Shared<ActivateFilterButton> button{"activatefilter"};
