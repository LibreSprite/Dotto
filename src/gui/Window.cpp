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


    void Window::focus(std::shared_ptr<ui::Node> child) {
        if (auto focus = focusTarget.lock()) {
            if (focus != child) {
                focusTarget = child->shared_from_this();
                focus->processEvent(ui::Blur{focus.get()});
                child->processEvent(ui::Focus{child.get()});
            }
        } else {
            focusTarget = child->shared_from_this();
            child->processEvent(ui::Focus{child.get()});
        }
    }

    void Window::blur(std::shared_ptr<ui::Node> child) {
        if (auto focus = focusTarget.lock()) {
            if (focus == child) {
                focusTarget.reset();
                focus->processEvent(ui::Blur{focus.get()});
            }
        }
    }

    void Window::on(msg::MouseMove& event) {
        if (event.windowId == id) {
            S32 eventX = event.x / scale;
            S32 eventY = event.y / scale;

            mouseButtons = event.buttons;
            mouseX = eventX;
            mouseY = eventY;

            if (hoverWindow == this) {
                if (auto target = dragTarget.lock()) {
                    if ((dragEvent.buttons & event.buttons) != dragEvent.buttons) {
                        pub(msg::EndDrag{});
                    } else {
                        dragEvent.x = dragEvent.anchorX + (eventX - dragEvent.initialX);
                        dragEvent.y = dragEvent.anchorY + (eventY - dragEvent.initialY);
                        dragEvent.globalX = eventX;
                        dragEvent.globalY = eventY;
                        target->processEvent(dragEvent);
                    }
                }
            }

            hoverWindow = this;
            ui::MouseMove guiEvent{nullptr, eventX, eventY, event.buttons};
            guiEvent.target = findEventTarget(guiEvent);
            if (!guiEvent.target)
                return;

            if (auto over = mouseOverTarget.lock()) {
                if (over.get() != guiEvent.target) {
                    mouseOverTarget = guiEvent.target->shared_from_this();
                    over->processEvent(ui::MouseLeave{over.get()});
                    guiEvent.target->processEvent(ui::MouseEnter{over.get()});
                }
            } else {
                mouseOverTarget = guiEvent.target->shared_from_this();
                guiEvent.target->processEvent(ui::MouseEnter{over.get()});
            }

            guiEvent.target->processEvent(guiEvent);
        }
    }

    void Window::on(msg::MouseDown& event) {
        if (event.windowId == id) {
            S32 eventX = event.x / scale;
            S32 eventY = event.y / scale;

            mouseButtons = event.buttons;
            mouseX = eventX;
            mouseY = eventY;
            hoverWindow = this;
            ui::MouseDown guiEvent{nullptr, eventX, eventY, event.buttons};
            guiEvent.target = findEventTarget(guiEvent);
            if (!guiEvent.target)
                return;

            focus(guiEvent.target->shared_from_this());

            if (guiEvent.target)
                guiEvent.target->processEvent(guiEvent);
        }
    }

    void Window::on(msg::MouseUp& event) {
        if (event.windowId == id) {
            S32 eventX = event.x / scale;
            S32 eventY = event.y / scale;

            mouseButtons = event.buttons;
            mouseX = eventX;
            mouseY = eventY;

            if (auto target = dragTarget.lock()) {
                if ((dragEvent.buttons & ~event.buttons) != dragEvent.buttons)
                    pub(msg::EndDrag{});
            }

            hoverWindow = this;

            ui::MouseUp guiEvent{nullptr, eventX, eventY, event.buttons};
            guiEvent.target = findEventTarget(guiEvent);
            if (!guiEvent.target)
                return;

            auto shared = guiEvent.target->shared_from_this();

            if (auto focus = focusTarget.lock()) {
                if (focus.get() == guiEvent.target) {
                    guiEvent.target->processEvent(ui::Click{focus.get(), eventX, eventY, event.buttons});
                }
            }

            guiEvent.target->processEvent(guiEvent);
        }
    }

    void Window::on(msg::KeyDown& event) {
        if (auto focus = focusTarget.lock()) {
            focus->processEvent(ui::KeyDown{focus.get(), event.scancode, event.keycode, event.keyName});
        }
    }

    void Window::on(msg::KeyUp& event) {
        if (auto focus = focusTarget.lock()) {
            focus->processEvent(ui::KeyUp{focus.get(), event.scancode, event.keycode, event.keyName});
        }
    }

    void Window::on(msg::WindowClosed& event) {
        if (event.windowId == id) {
            remove();
        }
    }

    void Window::on(msg::BeginDrag& event) {
        if (this == hoverWindow) {
            dragEvent.anchorX = event.anchorX;
            dragEvent.anchorY = event.anchorY;
            dragEvent.initialX = mouseX;
            dragEvent.initialY = mouseY;
            dragEvent.globalX = mouseX;
            dragEvent.globalY = mouseY;
            dragEvent.cancel = false;
            dragEvent.target = event.target.get();
            dragEvent.buttons = mouseButtons;
            dragTarget = event.target;
        }
    }

    void Window::on(msg::EndDrag& event) {
        if (this == hoverWindow) {
            if (auto target = dragTarget.lock())
                target->processEvent(ui::Drop{target.get(), dragEvent.globalX, dragEvent.globalY});
            dragTarget.reset();
        }
    }

    void Window::on(msg::PollActiveWindow& event) {
        if (this == hoverWindow)
            event.node = this;
    }

    ui::Node* Window::findEventTarget(const ui::Event& event) {
        ui::Node* target = event.target ?: this;
        if (!event.target && event.bubble != ui::Event::Bubble::Down) {
            while (event.target != target) {
                event.target = target;
                for (auto& child : target->getChildren()) {
                    if (child->visible && child->inputEnabled && child->globalRect.contains(event.globalX, event.globalY)) {
                        target = child.get();
                    }
                }
            }
        }
        return target;
    }
}
