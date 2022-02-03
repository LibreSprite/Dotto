// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class TabTarget : public ui::Controller {
public:
    void attach() override {
        node()->addEventListener<ui::KeyDown>(this);
        node()->set("IsTabTarget", true);
    }

    void eventHandler(const ui::KeyDown& event) {
        if (event.keyname != String("TAB"))
            return;
        inject<ui::Node> root{"root"};
        bool foundNode = false;
        std::shared_ptr<ui::Node> first, previous;
        auto next = root->findChildByPredicate([&](ui::Node* child){
                        auto& ps = child->getPropertySet();
                        if (!ps.get<bool>("IsTabTarget"))
                            return false;
                        if (!first)
                            first = child->shared_from_this();
                        if (child == node()) {
                            foundNode = true;
                            return false;
                        }
                        if (!foundNode)
                            previous = child->shared_from_this();
                        return foundNode;
                    });
        if (!next)
            next = first;
        if (next)
            next->focus();
    }
};

static ui::Controller::Shared<TabTarget> tabtarget{"tabtarget"};
