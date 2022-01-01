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
    PubSub<msg::MouseMove,
           msg::MouseUp,
           msg::MouseDown,
           msg::KeyDown,
           msg::KeyUp,
           msg::WindowClosed,
           msg::BeginDrag,
           msg::EndDrag,
           msg::PollActiveWindow> pub{this};
    std::weak_ptr<ui::Node> mouseOverTarget;
    std::weak_ptr<ui::Node> focusTarget;

    static inline ui::Window* hoverWindow = nullptr;
    static inline std::weak_ptr<ui::Node> dragTarget;
    static inline ui::Drag dragEvent{nullptr};
    static inline S32 mouseX = 0;
    static inline S32 mouseY = 0;
    static inline U32 mouseButtons = 0;

    ui::Node* findEventTarget(const ui::Event& event);

public:
    Property<String> title{this, "title"};
    Property<bool> maximized{this, "maximized"};
    Property<bool> border{this, "border"};
    Property<S32> x{this, "x"};
    Property<S32> y{this, "y"};
    Property<Color> background{this, "background"};
    Property<String> skin{this, "skin", "default"};

    void postInject() override;
    void resize() override;
    void doResize() override;
    void focus(std::shared_ptr<ui::Node> child) override;
    void blur(std::shared_ptr<ui::Node> child) override;

    void on(msg::MouseMove& event);
    void on(msg::MouseDown& event);
    void on(msg::MouseUp& event);
    void on(msg::KeyDown& event);
    void on(msg::KeyUp& event);
    void on(msg::WindowClosed& event);
    void on(msg::BeginDrag& event);
    void on(msg::EndDrag& event);
    void on(msg::PollActiveWindow& event);
};

}
