// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/String.hpp>
#include <doc/Cell.hpp>
#include <log/Log.hpp>
#include <filters/Filter.hpp>
#include <gui/Node.hpp>
#include <gui/Unit.hpp>

class ActivateFilter : public Command {
    Property<String> filter{this, "filter"};
    Property<bool> interactive{this, "interactive"};
    PubSub<> pub{this};
    std::weak_ptr<ui::Node> menu;

public:
    void run() override {
        auto it = Filter::instances.find(tolower(filter));
        if (it == Filter::instances.end()) {
            logE("Invalid filter \"", *filter, "\"");
            return;
        }

        auto& filter = it->second;
        if (Filter::active.lock() == filter) {
            logV("Filter already active");
            return;
        }

        inject<Cell> cell{"activecell"};
        if (!cell) {
            logE("No active cell");
            return;
        }

        if (!interactive) {
            filter->load(getPropertySet());
            filter->run(cell->getComposite()->shared_from_this());
            return;
        }

        auto metamenu = ui::Node::fromXML("metamenu");
        if (!metamenu) {
            logE("Could not create metamenu");
            return;
        }

        auto meta = filter->getMetaProperties();

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "row"},
                    {"id", "okcancel"},
                    {"height", "30px"}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "textbutton"},
                    {"parent", "okcancel"},
                    {"label", "cancel"},
                    {"click", FunctionRef<void()>([=]{
                        metamenu->remove();
                    })}
                }));

        auto shared = shared_from_this();
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "textbutton"},
                    {"parent", "okcancel"},
                    {"label", "ok"},
                    {"click", FunctionRef<void()>([=]{
                        logI("ok clicked");
                        auto widgets = metamenu->getPropertySet().get<std::shared_ptr<Vector<std::shared_ptr<ui::Node>>>>("widgets");
                        metamenu->remove();
                        if (!widgets)
                            return;
                        logI("got widgets");
                        PropertySet ps;
                        auto& map = ps.getMap();
                        for (auto widget : *widgets) {
                            auto label = widget->get("label");
                            auto result = widget->get("result");
                            auto value = widget->get("value");
                            if (result && result->has<String>()) {
                                auto parts = split(result->get<String>(), ".");
                                if (parts.size() == 2) {
                                    auto child = widget->findChildById(trim(parts[0]));
                                    value = child->get(parts[1]);
                                }
                            }
                            if (!label || !label->has<String>() || !value)
                                continue;
                            map[label->get<String>()] = value;
                            logV("Filter property [", label->get<String>(), "] = [", value->toString(), "]");
                        }
                        shared->load(ps);
                        interactive.value = false;
                        shared->run();
                    })}
                }));

        metamenu->set("meta", meta);
    }

};

static Command::Shared<ActivateFilter> cmd{"activatefilter"};
