// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/XML.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Node.hpp>


static ui::Node::Shared<ui::Node> node{"node"};

std::shared_ptr<ui::Node> ui::Node::fromXML(const String& widgetName) {
    auto& nodeRegistry = ui::Node::getRegistry();

    if (nodeRegistry.find(widgetName) != nodeRegistry.end())
        return inject<ui::Node>{widgetName};

    std::shared_ptr<XMLNode> xml = inject<FileSystem>{}->find("%appdata/gui/" + widgetName + ".xml")->parse();
    if (!xml || !xml->isElement())
        return nullptr;

    auto element = std::static_pointer_cast<XMLElement>(xml);
    auto widget = inject<ui::Node>{element->tag}.shared();
    if (!widget) widget = fromXML(element->tag);
    if (!widget) {
        logE("Unknown widget: ", element->tag);
        return nullptr;
    }

    {
        PropertySet props;
        for (auto& prop : element->attributes) {
            props.set(prop.first, prop.second);
        }
        widget->init(props);
    }

    for (auto xml : element->children) {
        if (!xml->isElement())
            continue;

        auto childElement = std::static_pointer_cast<XMLElement>(xml);
        auto child = fromXML(childElement->tag);
        if (!child)
            continue;

        {
            PropertySet props;
            for (auto& prop : childElement->attributes)
                props.set(prop.first, prop.second);
            child->init(props);
        }

        widget->addChild(child);
    }

    return widget;
}
