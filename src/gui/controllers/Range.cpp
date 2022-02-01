// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmath>
#include <common/FunctionRef.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Range : public ui::Controller {
public:
    PubSub<> pub{this};
    Property<F32> value{this, "value", 0.0f, &Range::changeValue};
    Property<F32> min{this, "min", 0.0f, &Range::changeValue};
    Property<F32> max{this, "max", 1.0f, &Range::changeValue};
    Property<F32> resolution{this, "resolution", 0.01f, &Range::changeValue};
    Property<bool> percent{this, "percent", false};
    std::weak_ptr<ui::Node> parent;

    ~Range() {
        if (auto parent = this->parent.lock())
            parent->removeEventListeners(this);
        if (auto node = this->node())
            node->removeEventListeners(this);
    }

    void changeValue() {
        F32 v = std::clamp(std::round(value / resolution) * resolution, *min, *max);
        if (v != value) {
            node()->set("value", v);
            return;
        }
        if (*min >= *max)
            return;
        v = ((v - min) * 100.0f) / (max - min);
        node()->set("x", ui::Unit{std::to_string(S32(v)) + "%-50%"});
    }

    void attach() override {
        node()->addEventListener<ui::Drag,
                                 ui::Drop,
                                 ui::Resize,
                                 ui::AddToScene>(this);
        changeValue();
    }

    void eventHandler(const ui::Resize&) {
        changeValue();
    }

    void eventHandler(const ui::AddToScene&) {
        if (auto parent = this->parent.lock())
            parent->removeEventListeners(this);
        auto parent = node()->getParent()->shared_from_this();
        this->parent = parent;
        parent->addEventListener<ui::MouseWheel,
                                 ui::MouseDown,
                                 ui::KeyDown>(this);
    }

    void eventHandler(const ui::KeyDown& event) {
        if (event.keyname == String("LEFT"))
            node()->set("value", value - resolution);
        if (event.keyname == String("RIGHT"))
            node()->set("value", value + resolution);
    }

    void eventHandler(const ui::MouseWheel& event) {
        F32 offset = event.wheelX + event.wheelY;
        F32 newValue = *value + *resolution * offset;
        value.value = newValue;
        changeValue();
    }

    void eventHandler(const ui::MouseDown&) {
        pub(msg::BeginDrag{
                node()->shared_from_this(),
                node()->globalRect.x,
                node()->globalRect.y
            });
    }

    void eventHandler(const ui::Drop& event) {
        node()->set("drag-value", "");
    }

    void eventHandler(const ui::Drag& event) {
        auto parent = node()->getParent();
        F32 width = parent->globalRect.width - parent->padding->x - parent->padding->width;
        F32 start = parent->globalRect.x + parent->padding->x;
        F32 value = std::clamp<F32>((event.x - start) / width * (max - min), min, max);
        if (value != this->value) {
            this->value.value = value;
            changeValue();
            value = this->value.value;
            node()->load({
                    {"value", value},
                    {"drag-value", percent ? std::to_string(S32(value*100)) + "%" : tostring(value) }
                });
            node()->processEvent(ui::Changed{node()});
        }
    }
};

static ui::Controller::Shared<Range> range{"range"};
