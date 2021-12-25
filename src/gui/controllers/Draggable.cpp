// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Unit.hpp>

class Draggable : public ui::Controller {
public:
    Property<String> dragHandle{this, "handle", "", &Draggable::changeTarget};
    std::weak_ptr<ui::Node> target;
    PubSub<> pub{this};

    ~Draggable() {
        if (auto child = target.lock())
            child->removeEventListeners(this);
    }

    void changeTarget() {
        if (auto child = target.lock())
            child->removeEventListeners(this);
        target.reset();
        if (node()) {
            auto child = dragHandle->empty() ? node()->shared_from_this() : node()->findChildById(dragHandle);
            target = child;
            if (child) {
                child->addEventListener<ui::MouseDown, ui::Drag, ui::Drop>(this);
            }
        }
    }

    void on(ui::Drop& event) {
    }

    void attach() override {
        if (!target.lock())
            changeTarget();
    }

    void eventHandler(const ui::MouseDown& event) {
        pub(msg::BeginDrag{
                target.lock(),
                node()->x->toPixel(node()->getParent()->localRect.width),
                node()->y->toPixel(node()->getParent()->localRect.height)
            });
    }

    void eventHandler(const ui::Drag& event) {
        node()->load({
                {"x", ui::Unit{event.x}},
                {"y", ui::Unit{event.y}}
            });
    }

    void eventHandler(const ui::Drop& event) {
    }

};

static ui::Controller::Shared<Draggable> draggable{"draggable"};
