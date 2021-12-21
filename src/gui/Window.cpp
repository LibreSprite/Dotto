// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <gui/Events.hpp>
#include <gui/Window.hpp>

namespace ui {

    void Window::postInject() {
        if (auto root = inject<ui::Node>{"root"})
            root->addChild(shared_from_this());
    }

    void Window::resize() {
        needResize = true;
    }

    void Window::doResize() {
        needResize = false;
        Node::doResize();
    }

    void Window::on(msg::MouseMove& event) {
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

    void Window::on(msg::MouseDown& event) {
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

    void Window::on(msg::MouseUp& event) {
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

    void Window::on(msg::KeyDown& event) {
        if (auto focus = focusTarget.lock()) {
            focus->processEvent(ui::KeyDown{event.scancode, event.keycode});
        }
    }

    void Window::on(msg::KeyUp& event) {
        if (auto focus = focusTarget.lock()) {
            focus->processEvent(ui::KeyUp{event.scancode, event.keycode});
        }
    }

    void Window::on(msg::WindowClosed& event) {
        if (event.windowId == id) {
            remove();
        }
    }

    ui::Node* Window::findEventTarget(const ui::Event& event) {
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
}
