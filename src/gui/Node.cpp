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

    std::shared_ptr<XMLNode> xml = inject<FileSystem>{}->parse("%appdata/gui/" + widgetName + ".xml");
    if (!xml || !xml->isElement())
        return nullptr;

    auto element = std::static_pointer_cast<XMLElement>(xml);
    auto widget = inject<ui::Node>{element->tag}.shared();
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
