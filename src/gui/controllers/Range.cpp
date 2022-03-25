// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmath>
#include <common/Config.hpp>
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
    inject<Config> config;
    PubSub<> pub{this};
    F64 oldValue = std::nan("");
    Property<F64> value{this, "value", 0.0, &Range::changeValue};
    Property<F64> min{this, "min", 0.0, &Range::changeValue};
    Property<F64> max{this, "max", 1.0, &Range::changeValue};
    Property<F64> resolution{this, "resolution", 0.01, &Range::changeValue};
    Property<bool> percent{this, "percent", false};
    Property<String> handleName{this, "handle", "", &Range::setHandle};
    Property<String> format{this, "format", "", &Range::changeFormat};
    std::shared_ptr<ui::Node> handle;
    bool forceUpdate;

    ~Range() {
        if (handle)
            handle->removeEventListeners(this);
    }

    void setHandle() {
        if (handle)
            handle->removeEventListeners(this);
        handle = node()->findChildById(handleName);
        if (handle) {
            handle->addEventListener<ui::Drag,
                                     ui::Drop,
                                     ui::MouseDown,
                                     ui::Resize>(this);
        }
    }

    void changeFormat() {
        forceUpdate = true;
        changeValue();
    }

    void changeValue() {
        if (resolution == 0)
            *resolution = 1;
        F64 v = std::clamp(std::round(value / resolution) * resolution, *min, *max);
        if (v != value) {
            node()->set("value", v);
            return;
        }
        if (*min >= *max)
            return;
        if (handle) {
            v = ((v - min) * 100.0) / (max - min);
            handle->set("x", ui::Unit{std::to_string(S32(v)) + "%"});
        }

        if (value != oldValue) {
            forceUpdate = true;
            oldValue = value;
            node()->processEvent(ui::Changed{node()});
        }

        if (!format->empty() && forceUpdate) {
            String format = config->translate(this->format, node());
            v = *value;
            if (percent) v *= 100;
            String str = S32(resolution) == *resolution ? std::to_string(S32(v)) : tostring(v);
            node()->set("label", std::regex_replace(str.c_str(), std::regex("(.+)"), format.c_str()));
        }

        forceUpdate = false;
    }

    void attach() override {
        node()->addEventListener<ui::MouseWheel,
                                 ui::KeyDown>(this);
        changeValue();
    }

    void eventHandler(const ui::Resize&) {
        changeValue();
    }

    void eventHandler(const ui::KeyDown& event) {
        if (event.keyname == String("LEFT"))
            node()->set("value", value - resolution);
        if (event.keyname == String("RIGHT"))
            node()->set("value", value + resolution);
    }

    void eventHandler(const ui::MouseWheel& event) {
        F64 offset = event.wheelX + event.wheelY;
        F64 newValue = *value + *resolution * offset;
        node()->set("value", newValue);
    }

    void eventHandler(const ui::MouseDown&) {
        pub(msg::BeginDrag{
                handle->shared_from_this(),
                handle->globalRect.x,
                handle->globalRect.y
            });
    }

    void eventHandler(const ui::Drop& event) {
        node()->set("drag-value", "");
    }

    void eventHandler(const ui::Drag& event) {
        F64 width = node()->globalRect.width - node()->padding->x - node()->padding->width;
        F64 start = node()->globalRect.x + node()->padding->x;
        F64 value = std::clamp<F64>((event.x - start) / width * (max - min) + min, min, max);
        if (value != this->value) {
            this->value.value = value;
            changeValue();
            value = this->value.value;
            String drag;
            if (S32(resolution * (percent ? 100 : 1)) == resolution * (percent ? 100 : 1)) {
                if (percent) {
                    drag = std::to_string(S32(value * 100)) + "%";
                } else {
                    drag = std::to_string(S32(value));
                }
            } else {
                if (percent) {
                    drag = tostring(value * 100) + "%";
                } else {
                    drag = tostring(value);
                }
            }
            node()->load({
                    {"value", tostring(value)},
                    {"drag-value", drag}
                });
        }
    }
};

static ui::Controller::Shared<Range> range{"range"};
