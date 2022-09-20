// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PropertySet.hpp>
#include <common/PubSub.hpp>
#include <common/String.hpp>
#include <common/types.hpp>
#include <filters/Filter.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class MetaMenu : public ui::Controller {
public:
    PubSub<msg::InvalidateMetaMenu> pub{this};

    Property<std::shared_ptr<Vector<std::shared_ptr<ui::Node>>>> widgets{this, "widgets"};
    Property<std::shared_ptr<PropertySet>> meta{this, "meta", nullptr, &MetaMenu::changeMeta};
    Property<std::shared_ptr<PropertySet>> result{this, "result", std::make_shared<PropertySet>(), &MetaMenu::readResult};
    Property<String> containerId{this, "container"};
    bool ignoreChange = false;
    void changeMeta() {
        if (*meta)
            build();
    }

    void on(msg::InvalidateMetaMenu& event) {
        if (event.oldMeta != *meta)
            return;
        if (ignoreChange)
            *meta = event.newMeta;
        set("meta", event.newMeta);
        ignoreChange = false;
    }

    void readResult() {
        if (!*result) {
            result.value = std::make_shared<PropertySet>();
            node()->set("result", result.value);
        }
        ignoreChange = true;
        PropertySet& ps = **result;
        if (!*widgets)
            return;
        auto& map = ps.getMap();
        for (auto widget : **widgets) {
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
        }
    }

    void build() {
        if (!node()) {
            logV("Bail, no node");
            return;
        }

        widgets.value = std::make_shared<Vector<std::shared_ptr<ui::Node>>>();
        Vector<std::shared_ptr<ui::Node>> rows;
        S32 height = 0, width = 300;

        if (*this->meta) {
            HashMap<String, std::shared_ptr<ui::Node>> map;
            auto tags = this->node()->getTags();
            auto& meta = this->meta.value->getMap();
            for (std::size_t i = 0, size = this->meta.value->size(); i < size; ++i) {
                auto it = meta.find(std::to_string(i));
                if (it == meta.end() || !it->second->has<std::shared_ptr<PropertySet>>())
                    continue;
                auto descriptor = it->second->get<std::shared_ptr<PropertySet>>();
                auto widgetName = descriptor->get<String>("widget");
                if (widgetName.empty()) {
                    logE("Descriptor ", i, " without \"widget\" key.");
                    continue;
                }
                auto node = ui::Node::fromXML(widgetName);
                auto node = ui::Node::fromXML(widgetName, tags);
                if (!node) {
                    logE("Could not create widget \"", widgetName, "\" for meta property.");
                    continue;
                }
                node->load(*descriptor);
                widgets.value->push_back(node);
                if (!node->id->empty())
                    map[*node->id] = node;
                auto parent = node->getPropertySet().get<String>("parent");
                if (!parent.empty()) {
                    auto it = map.find(parent);
                    if (it != map.end()) {
                        it->second->addChild(node);
                        continue;
                    }
                }
                rows.push_back(node);
            }
        }

        for (auto& node : rows) {
            height += node->height->toPixel(0, 0);
            height += node->margin->y + node->margin->height;
            width = std::max(node->width->toPixel(width, 0), width);
        }

        auto container = containerId->empty() ? node()->shared_from_this() : node()->findChildById(containerId);
        if (!container) {
            logI("Could not find [", *containerId, "]");
            container = node()->shared_from_this();
        }

        container->removeAllChildren();
        for (auto& row : rows) {
            container->addChild(row);
        }
        container->load({
                {"width", width},
                {"height", height}
            });

        while (container && container.get() != node()) {
            auto& properties = container->getPropertySet();
            auto padding = properties.get<Rect>("padding");
            height += padding.y + padding.height;
            width += padding.x + padding.width;
            auto margin = properties.get<Rect>("margin");
            height += margin.y + margin.height;
            width += margin.x + margin.width;
            container = container->getParent()->shared_from_this();
        }

        auto& properties = node()->getPropertySet();
        width += properties.get<ui::Unit>("extra-width").toPixel(0,0);
        height += properties.get<ui::Unit>("extra-height").toPixel(0,0);
        auto padding = properties.get<Rect>("padding");
        height += padding.y + padding.height;
        width += padding.x + padding.width;

        node()->set("width", width);
        node()->set("height", height);
        node()->set("widgets", widgets.value);
    }
};

static ui::Controller::Shared<MetaMenu> metamenu{"metamenu"};
