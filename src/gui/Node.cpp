// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/XML.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Flow.hpp>
#include <gui/Graphics.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>


static ui::Node::Shared<ui::Node> node{"node"};


ui::Node::Node() {
    addEventListener<AddToScene, RemoveFromScene, Focus, Blur>(this);
    loadSilent({{"node", this}});
}

ui::Node::~Node() {
    // logV("Deleting node ", *id);
    for (auto& child : children)
        child->parent = nullptr;
}


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

void ui::Node::processEvent(const Event& event) {
    EventHandler::processEvent(event);
    if (event.bubble == Event::Bubble::Down && !event.cancel)
        forwardToChildren(event);
    if (event.bubble == Event::Bubble::Up && !event.cancel && parent)
        parent->processEvent(event);
}

void ui::Node::setDirty() {
    if (!isDirty) {
        isDirty = true;
        if (parent)
            parent->setDirty();
    }
}

bool ui::Node::init(const PropertySet& properties) {
    load(properties);
    reflow();
    return true;
}

bool ui::Node::update() {
    if (!isDirty)
        return false;
    isDirty = false;
    for (auto& child : children) {
        child->update();
    }
    return true;
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

void ui::Node::reflow() {
    flowInstance = inject<Flow>{*flow};
    setDirty();
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

void ui::Node::forwardToChildren(const Event& event) {
    for (auto& child : children)
        child->processEvent(event);
}

void ui::Node::eventHandler(const AddToScene& event) {
    isInScene = true;
    resize();
    if (isDirty && parent) // parent is null for root node
        parent->setDirty();
}

void ui::Node::doResize() {
    if (!parent || !flowInstance)
        return;
    auto innerRect = globalRect;
    innerRect.x += padding->x;
    innerRect.y += padding->y;
    innerRect.width -= padding->x + padding->width;
    innerRect.height -= padding->y + padding->height;
    flowInstance->update(children, innerRect);
    for (auto& child : children)
        child->doResize();
}

void ui::Node::draw(S32 z, Graphics& gfx) {
    if (*hideOverflow) {
        Rect clip = gfx.pushClipRect(globalRect);
        if (!gfx.isEmptyClipRect()) {
            for (auto& child : children) {
                if (child->visible)
                    child->draw(z + 1 + *child->zIndex, gfx);
            }
        }
        gfx.setClipRect(clip);
    } else {
        for (auto& child : children) {
            if (child->visible)
                child->draw(z + 1 + *child->zIndex, gfx);
        }
    }
}

void ui::Node::addChild(std::shared_ptr<Node> child) {
    if (!child)
        return;
    child->remove();
    children.push_back(child);
    child->parent = this;
    if (isInScene)
        child->processEvent(AddToScene{child.get()});
}

void ui::Node::removeChild(std::shared_ptr<Node> node) {
    if (!node) return;
    if (node->parent == this) {
        auto it = std::find(children.begin(), children.end(), node);
        if (it != children.end()) {
            node->parent = nullptr;
            children.erase(it);
            node->processEvent(RemoveFromScene{node.get()});
        }
    }
}
