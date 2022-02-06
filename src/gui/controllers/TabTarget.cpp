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
        event.cancel = true;
        inject<ui::Node> root{"root"};
        bool foundNode = false;
        std::shared_ptr<ui::Node> first, previous, next, last;
        root->findChildByPredicate([&](ui::Node* child){
            auto& ps = child->getPropertySet();
            if (!ps.get<bool>("IsTabTarget"))
                return false;
            last = child->shared_from_this();
            if (!first)
                first = last;
            if (child == node()) {
                foundNode = true;
                return false;
            }
            if (!foundNode)
                previous = last;
            else if (!next)
                next = last;
            return false;
        });
        if (!next)
            next = first;
        if (!previous)
            previous = last;
        if (event.pressedKeys.find("LSHIFT") != event.pressedKeys.end())
            next = previous;
        if (next)
            next->focus();
    }
};

static ui::Controller::Shared<TabTarget> tabtarget{"tabtarget"};
