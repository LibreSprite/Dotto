// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <map>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <filters/Filter.hpp>

class Filters : public ui::Controller {
    PubSub<msg::BootComplete, msg::Shutdown> pub{this};
    Vector<std::shared_ptr<ui::Node>> buttonPool;
    Vector<std::shared_ptr<ui::Node>> labelPool;
    HashMap<Filter*, std::shared_ptr<ui::Node>> filterButton;

public:
    void on(msg::BootComplete&) {
        update();
    }

    void on(msg::Shutdown&) {
        Filter::instances.clear();
    }

    void update() {
        using Entry = std::pair<const String, std::shared_ptr<Filter>>;
        std::map<String, Vector<Entry*>> enabled;
        for (auto& entry : Filter::instances) {
            if (entry.second->enabled) {
                enabled[entry.second->category()].push_back(&entry);
            }
        }

        for (auto& set : enabled) {
            std::sort(set.second.begin(), set.second.end(), [](auto left, auto right) {
                return left->first < right->first;
            });
        }

        for (auto it = filterButton.begin(); it != filterButton.end();) {
            auto filter = it->first;
            auto& added = it->second;
            added->remove();

            bool found = false;
            for (auto& set : enabled) {
                for (auto entry : set.second) {
                    if (entry->second.get() == filter) {
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;
            }

            if (found) {
                ++it;
            } else {
                buttonPool.push_back(added);
                it = filterButton.erase(it);
            }
        }

        S32 height = 0;

        for (auto& set : enabled) {
            auto category = ui::Node::fromXML("filtercategory");
            S32 categoryHeight = category->height->toPixel(0, 0);
            height += categoryHeight;
            node()->addChild(category);

            for (auto entry : set.second) {
                std::shared_ptr<ui::Node> button;
                auto filter = entry->second;
                auto it = filterButton.find(entry->second.get());
                if (it != filterButton.end()) {
                    button = it->second;
                } else {
                    if (buttonPool.empty()) {
                        button = ui::Node::fromXML("filter");
                    } else {
                        button = buttonPool.back();
                        buttonPool.pop_back();
                    }
                    if (!button) {
                        logE("Could not create filter button");
                        return;
                    }
                    filterButton[filter.get()] = button;
                    button->load(entry->second->getPropertySet());
                }
                category->addChild(button);
                S32 buttonHeight = button->height->toPixel(0, 0);
                height += buttonHeight;
                categoryHeight += buttonHeight;
            }

            category->load({
                    {"label", set.first},
                    {"height", categoryHeight}
                });
        }
    }

    void attach() override {
        Filter::boot();
    }
};

static ui::Controller::Shared<Filters> filterlist{"filterlist"};
