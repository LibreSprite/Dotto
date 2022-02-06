// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <tools/Tool.hpp>

class ToolBox : public ui::Controller {
    PubSub<msg::BootComplete, msg::Shutdown> pub{this};
    Vector<std::shared_ptr<ui::Node>> buttonPool;
    HashMap<Tool*, std::shared_ptr<ui::Node>> toolButton;
    Property<String> containerId{this, "container", "", &ToolBox::setContainer};
    Property<S32> extraHeight{this, "extra-height", 0};
    std::weak_ptr<ui::Node> container;

public:
    void setContainer() {
        container = node()->findChildById(containerId);
    }

    void on(msg::BootComplete&) {
        update();
    }

    void on(msg::Shutdown&) {
        Tool::instances.clear();
    }

    void update() {
        auto container = this->container.lock();
        if (!container)
            container = node()->shared_from_this();

        using Entry = std::pair<const String, std::shared_ptr<Tool>>;
        Vector<Entry*> enabled;
        for (auto& entry : Tool::instances) {
            if (entry.second->enabled) {
                enabled.push_back(&entry);
            }
        }

        std::sort(enabled.begin(), enabled.end(), [](auto left, auto right) {
            return left->first < right->first;
        });

        for (auto it = toolButton.begin(); it != toolButton.end();) {
            auto tool = it->first;
            auto& added = it->second;
            added->remove();
            bool found = false;
            for (auto entry : enabled) {
                if (entry->second.get() == tool) {
                    found = true;
                    break;
                }
            }
            if (found) {
                ++it;
            } else {
                buttonPool.push_back(added);
                it = toolButton.erase(it);
            }
        }

        U32 height = *extraHeight + node()->padding->y + node()->padding->height;
        for (auto entry : enabled) {
            std::shared_ptr<ui::Node> button;
            auto tool = entry->second;
            auto it = toolButton.find(entry->second.get());
            if (it != toolButton.end()) {
                button = it->second;
            } else {
                if (buttonPool.empty()) {
                    button = ui::Node::fromXML("tool");
                } else {
                    button = buttonPool.back();
                    buttonPool.pop_back();
                }
                if (!button) {
                    logE("Could not create tool button");
                    return;
                }
                toolButton[tool.get()] = button;
                button->load(entry->second->getPropertySet());
            }
            height += button->outerHeight();
            container->addChild(button);
        }

        node()->set("height", height);
    }

    void attach() override {
        Tool::boot();
    }
};

static ui::Controller::Shared<ToolBox> toolbox{"toolbox"};
