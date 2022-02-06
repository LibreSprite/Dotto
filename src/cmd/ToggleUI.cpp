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

public:
    void run() override {
        inject<ui::Node> root{"root"};
        if (root) {
            if (auto node = root->findChildById(id)) {
                auto& ps = node->getPropertySet();
                auto current = ps.get<bool>(property);

                if (!hide->empty() && !current) {
                    logI("Searching for ", *hide);
                    root->findChildByPredicate([&](ui::Node* child){
                        if (child->getPropertySet().get<String>("class") == *hide) {
                            child->set(property, false);
                        }
                        return false;
                    });
                }

                node->set(property, !current);
                if (bringToFront)
                    node->bringToFront();
            } else {
                logE("Could not find ", id, " for toggle");
            }
        }
    }
};

static Command::Shared<ToggleUI> cmd{"toggleui"};
