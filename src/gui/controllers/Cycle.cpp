// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Cycle : public ui::Controller {
public:
    Property<String> value{this, "value"};
    Property<String> options{this, "options"};

    Vector<String> getOptions() {
        Vector<String> strs = split(options, ",");
        for (auto& str : strs) {
            str = trim(str);
        }
        return strs;
    }

    void attach() override {
        node()->addEventListener<ui::Click>(this);
        auto options = getOptions();
        if (!options.empty()) {
            auto it = std::find(options.begin(), options.end(), *value);
            if (it == options.end()) {
                node()->load({{"value", options.front()}});
            }
        }
    }

    void eventHandler(const ui::Click&) {
        auto options = getOptions();
        auto it = std::find(options.begin(), options.end(), *value);
        String newValue;
        if (it == options.end() || it + 1 == options.end()) {
            newValue = options.front();
        } else {
            newValue = *++it;
        }
        node()->load({{"value", newValue}});
        node()->processEvent(ui::Changed{node()});
    }
};

static ui::Controller::Shared<Cycle> reg{"cycle"};
