// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/FunctionRef.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class DropDown : public ui::Controller {
public:
    Property<String> value{this, "value"};
    Property<String> options{this, "options", "", &DropDown::invalidateContainer};

    std::shared_ptr<ui::Node> container;

    void invalidateContainer() {
        if (container)
            container->remove();
        container.reset();
    }

    std::shared_ptr<ui::Node> initContainer() {
        if (container)
            return container;
        container = ui::Node::fromXML("dropdowncontainer");

        for (auto& opt : getOptions()) {
            auto entry = ui::Node::fromXML("menubutton");
            entry->load({
                    {"label", opt},
                    {"click", FunctionRef<void()>([=]{
                        logI("Click ", opt);
                        if (*value != opt) {
                            container->remove();
                            node()->load({{"value", opt}});
                            node()->processEvent(ui::Changed{node()});
                        }
                    })}
                });
            container->addChild(entry);
        }

        return container;
    }

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
        if (container && container->getParent()) {
            container->remove();
            logI("Removed container");
        } else {
            if (!container) {
                container = initContainer();
                logI("created container");
            }

            auto parent = node();
            ui::Node* window = nullptr;
            while (parent) {
                if (parent->hasTag("window"))
                    window = parent;
                parent = parent->getParent();
            }

            if (!window) {
                logE("Could not find window for container");
                return;
            }

            auto rect = node()->globalRect;
            logI(rect);
            logI("x", std::to_string(rect.left()) + "px");
            logI("y", std::to_string(rect.bottom()) + "px");
            logI("width", std::to_string(rect.width) + "px");

            container->load({
                    {"x", std::to_string(rect.left()) + "px"},
                    {"y", std::to_string(rect.bottom()) + "px"},
                    {"width", std::to_string(rect.width) + "px"}
                });
            window->addChild(container);
            logI("added container");
        }
    }
};

static ui::Controller::Shared<DropDown> reg{"dropdown"};
