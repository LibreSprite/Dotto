// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/XML.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Node.hpp>


static ui::Node::Shared<ui::Node> node{"node"};

std::shared_ptr<ui::Node> ui::Node::findChildById(const String& targetId) {
    if (*id == targetId)
        return shared_from_this();
    for (auto& child : children) {
        if (auto found = child->findChildById(targetId))
            return found;
    }
    return nullptr;
}

void ui::Node::bringToFront(std::shared_ptr<ui::Node> child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it == children.end())
        return;
    children.erase(it);
    children.push_back(child);
}

U32 ui::Node::getChildSeparation(std::shared_ptr<ui::Node> child) {
    if (!child)
        return 0;
    auto current = child->parent;
    U32 depth = 1;
    while (current) {
        if (current == this)
            return depth;
        depth++;
        current = current->parent;
    }
    return 0;
}

void ui::Node::load(const PropertySet& set) {
    Model::load(set);
    for (auto& entry : controllers) {
        entry.second->init(set);
    }
    if (!forward->empty()) {
        for (const auto& forward : split(this->forward, ",")) {
            auto parts = split(forward, "=");
            if (parts.size() != 2)
                continue;

            Value value;
            if (!set.get(trim(parts[1]), value))
                continue;

            auto target = split(parts[0], ".");
            if (target.size() != 2)
                continue;

            auto child = findChildById(trim(target[0]));
            if (!child)
                continue;

            child->load({{target[1], value}});
        }
    }
}

void ui::Node::set(const String& key, Value& value, bool debug) {
    Model::set(key, value, debug);
    for (auto& entry : controllers) {
        entry.second->set(key, value, debug);
    }
    if (!forward->empty()) {
        for (const auto& forward : split(this->forward, ",")) {
            auto parts = split(forward, "=");
            if (parts.size() != 2)
                continue;

            if (key != trim(parts[1]))
                continue;

            auto target = split(parts[0], ".");
            if (target.size() != 2)
                continue;

            auto child = findChildById(trim(target[0]));
            if (!child)
                continue;

            child->set(target[1], value, debug);
        }
    }
}

static void loadNodeProperties(ui::Node* node, XMLElement* element) {
    PropertySet props;
    if (!element->text.empty())
        props.set("text", trim(element->text));
    for (auto& prop : element->attributes) {
        props.set(prop.first, prop.second);
    }
    node->init(props);
}

static void loadChildNodes(ui::Node* parent, XMLElement* element) {
    for (auto xml : element->children) {
        if (!xml->isElement())
            continue;

        auto childElement = std::static_pointer_cast<XMLElement>(xml);
        auto child = parent->findChildById(childElement->tag);

        if (!child) {
            child = ui::Node::fromXML(childElement->tag);
            if (child) {
                parent->addChild(child);
            }
        }

        if (!child)
            continue;

        loadChildNodes(child.get(), childElement.get());
        loadNodeProperties(child.get(), childElement.get());
    }
}

std::shared_ptr<ui::Node> ui::Node::fromXML(const String& widgetName) {
    auto& nodeRegistry = ui::Node::getRegistry();

    if (nodeRegistry.find(widgetName) != nodeRegistry.end())
        return inject<ui::Node>{widgetName};

    inject<FileSystem> fs;
    std::shared_ptr<XMLNode> xml = fs->parse("%skin/gui/" + widgetName + ".xml");
    if (!xml || !xml->isElement())
        xml = fs->parse("%appdata/skins/default/gui/" + widgetName + ".xml");
    if (!xml || !xml->isElement())
        return nullptr;

    auto element = std::static_pointer_cast<XMLElement>(xml);
    auto widget = inject<ui::Node>{InjectSilent::Yes, element->tag}.shared();
    if (!widget) widget = fromXML(element->tag);
    if (!widget) {
        logE("Unknown widget: ", element->tag);
        return nullptr;
    }

    loadChildNodes(widget.get(), element.get());
    loadNodeProperties(widget.get(), element.get());

    return widget;
}

void ui::Node::reattach() {
    for (auto name : split(controllerName, ",")) {
        auto clean = trim(name);
        if (clean.empty() || controllers.find(clean) != controllers.end())
            continue;
        if (auto controller = inject<Controller>{clean}.shared()) {
            controllers[clean] = controller;
            controller->init(getPropertySet());
        }
    }
}
