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

class ToggleUI : public Command {
    Property<String> id{this, "id"};
    Property<String> hide{this, "hide"};
    Property<bool> bringToFront{this, "bringtofront"};
    Property<String> property{this, "property", "visible"};
    Property<String> mode{this, "mode", "toggle"};

public:
    void run() override {
        inject<ui::Node> root{"root"};
        if (!root)
            return;

        auto node = root->findChildById(id);
        if (!node) {
            logE("Could not find ", id, " for toggle");
            return;
        }

        auto& ps = node->getPropertySet();
        bool current = false;

        if (*mode == "toggle") {
            current = !ps.get<bool>(property);
        } else if (*mode == "set") {
            current = true;
        } else if (*mode == "clear") {
            current = false;
        } else {
            logE("Invalid mode for ToggleUI: ", *mode);
            return;
        }

        if (hide->empty() && current) {
            *hide = node->getPropertySet().get<String>("class");
        }

        if (!hide->empty() && current) {
            root->findChildByPredicate([&](ui::Node* child){
                if (child->getPropertySet().get<String>("class") == *hide) {
                    child->set(property, false);
                }
                return false;
            });
        }

        node->set(property, current);
        if (bringToFront)
            node->bringToFront();
    }
};

static Command::Shared<ToggleUI> cmd{"toggleui"};
