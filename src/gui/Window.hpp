// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Color.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

namespace ui {

class Window : public Node {
protected:
    bool needResize = true;
    U32 id;
    PubSub<msg::MouseMove, msg::MouseUp, msg::MouseDown> pub{this};
    inline static std::weak_ptr<ui::Node> mouseOverTarget;
    inline static std::weak_ptr<ui::Node> focusTarget;

public:
    Property<String> title{this, "title"};
    Property<bool> maximized{this, "maximized"};
    Property<bool> border{this, "border"};
    Property<S32> x{this, "x"};
    Property<S32> y{this, "y"};
    Property<Color> background{this, "background"};
    Property<String> skin{this, "skin", "default"};

    void postInject() override {
        if (auto root = inject<ui::Node>{"root"})
            root->addChild(shared_from_this());
    }

    void resize() override {
        needResize = true;
    }

    void doResize() override {
        needResize = false;
        Node::doResize();
    }

    void on(msg::MouseMove& event) {
        if (event.windowId == id) {
            ui::MouseMove guiEvent{event.x, event.y, event.buttons};
            guiEvent.target = findEventTarget(guiEvent);
            if (!guiEvent.target)
                return;

            if (auto over = mouseOverTarget.lock()) {
                if (over.get() != guiEvent.target) {
                    mouseOverTarget = guiEvent.target->shared_from_this();
                    over->processEvent(ui::MouseLeave{});
                    guiEvent.target->processEvent(ui::MouseEnter{});
                }
            } else {
                mouseOverTarget = guiEvent.target->shared_from_this();
                guiEvent.target->processEvent(ui::MouseEnter{});
            }

            guiEvent.target->processEvent(guiEvent);
        }
    }

    void on(msg::MouseDown& event) {
        if (event.windowId == id) {
            ui::MouseDown guiEvent{event.x, event.y, event.buttons};
            guiEvent.target = findEventTarget(guiEvent);
            if (!guiEvent.target)
                return;

            if (auto focus = focusTarget.lock()) {
                if (focus.get() != guiEvent.target) {
                    focusTarget = guiEvent.target->shared_from_this();
                    focus->processEvent(ui::Blur{});
                    guiEvent.target->processEvent(ui::Focus{});
                }
            } else {
                focusTarget = guiEvent.target->shared_from_this();
                guiEvent.target->processEvent(ui::Focus{});
            }

            if (guiEvent.target)
                guiEvent.target->processEvent(guiEvent);
        }
    }

    void on(msg::MouseUp& event) {
        if (event.windowId == id) {
            ui::MouseUp guiEvent{event.x, event.y, event.buttons};
            guiEvent.target = findEventTarget(guiEvent);
            if (!guiEvent.target)
                return;

            if (auto focus = focusTarget.lock()) {
                if (focus.get() == guiEvent.target) {
                    guiEvent.target->processEvent(ui::Click{event.x, event.y, event.buttons});
                }
            }

            guiEvent.target->processEvent(guiEvent);
        }
    }

    ui::Node* findEventTarget(const ui::Event& event) {
        ui::Node* target = event.target ?: this;
        if (!event.target && event.bubble != ui::Event::Bubble::Down) {
            while (event.target != target) {
                event.target = target;
                for (auto& child : target->getChildren()) {
                    if (child->globalRect.contains(event.globalX, event.globalY)) {
                        target = child.get();
                    }
                }
            }
        }
        return target;
    }
};

}
