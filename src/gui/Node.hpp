// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>
#include <gui/Events.hpp>
#include <gui/EventHandler.hpp>
#include <gui/Flow.hpp>
#include <gui/Graphics.hpp>
#include <gui/Rect.hpp>
#include <gui/Unit.hpp>
#include <log/Log.hpp>

namespace ui {

    class Controller;

    class Node : public Injectable<Node>,
                 public EventHandler,
                 public Serializable,
                 public std::enable_shared_from_this<Node> {
        friend class EventHandler;
        PropertySet model{{"node", this}};
        Vector<std::shared_ptr<Node>> children;
        Node* parent = nullptr;
        bool isInScene = false;
        bool isDirty = true;
        std::shared_ptr<Flow> flowInstance;
        std::shared_ptr<Controller> widget;
        std::shared_ptr<Controller> controller;

        void reflow() {
            flowInstance = inject<Flow>{*flow};
            setDirty();
        }

        void reattach();
        void reattachWidget();

    protected:
        void forwardToChildren(const Event& event) {
            for (auto& child : children)
                child->processEvent(event);
        }

        virtual void eventHandler(const AddToScene& event) {
            isInScene = true;
            resize();
            if (isDirty && parent) // parent is null for root node
                parent->setDirty();
        }

        virtual void eventHandler(const RemoveFromScene& event) {
            isInScene = false;
        }

    public:
        Rect localRect, globalRect;

        Property<String> id{this, "id", ""};
        Property<String> widgetName{this, "widget", "", &Node::reattachWidget};
        Property<String> controllerName{this, "controller", "", &Node::reattach};
        Property<bool> visible{this, "visible"};

        Property<bool> hideOverflow{this, "overflow-hidden", false};
        Property<bool> absolute{this, "absolute", false, &Node::resize};
        Property<Unit> x{this, "x", {"0px"}, &Node::resize};
        Property<Unit> y{this, "y", {"0px"}, &Node::resize};

        Property<Unit> width{this, "width", {"50px"}, &Node::resize};
        Property<Unit> minWidth{this, "min-width", {"10px"}, &Node::resize};
        Property<Unit> maxWidth{this, "max-width", {"100%"}, &Node::resize};

        Property<Unit> height{this, "height", {"50px"}, &Node::resize};
        Property<Unit> minHeight{this, "min-height", {"10px"}, &Node::resize};
        Property<Unit> maxHeight{this, "max-height", {"100%"}, &Node::resize};

        Property<String> flow{this, "flow", "column", &Node::reflow};
        Property<S32> zIndex{this, "z", 0};

        Node() {
            addEventListener<AddToScene, RemoveFromScene>(this);
        }

        static std::shared_ptr<Node> fromXML(const String& widgetName);

        virtual bool init(const PropertySet& properties) {
            model.append(properties);
            load(properties);
            reflow();
            return true;
        }

        std::shared_ptr<Node> findChildById(const String& targetId);

        void set(const String& key, Value& value) {
            model.set(key, value);
            Serializable::set(key, value);
        }

        void set(const String& key, const Value& value) {
            Value copy = value;
            model.set(key, value);
            Serializable::set(key, copy);
        }

        virtual void processEvent(const Event& event) {
            EventHandler::processEvent(event);
            if (event.bubble == Event::Bubble::Down && !event.cancel)
                forwardToChildren(event);
            if (event.bubble == Event::Bubble::Up && !event.cancel && parent)
                parent->processEvent(event);
        }

        virtual void setDirty() {
            if (!isDirty) {
                isDirty = true;
                if (parent)
                    parent->setDirty();
            }
        }

        const Vector<std::shared_ptr<Node>>& getChildren() {
            return children;
        }

        virtual bool update() {
            if (!isDirty)
                return false;
            isDirty = false;
            for (auto& child : children) {
                child->update();
            }
            return true;
        }

        virtual void resize() {
            if (parent)
                parent->resize();
        }

        virtual void doResize() {
            if (!parent || !flowInstance)
                return;
            flowInstance->update(children, globalRect);
            for (auto& child : children)
                child->doResize();
        }

        virtual void draw(S32 z, Graphics& gfx) {
            if (*hideOverflow) {
                Rect clip = gfx.pushClipRect(globalRect);
                if (!gfx.isEmptyClipRect()) {
                    for (auto& child : children)
                        child->draw(z + 1 + *child->zIndex, gfx);
                }
                gfx.setClipRect(clip);
            } else {
                for (auto& child : children)
                    child->draw(z + 1 + *child->zIndex, gfx);
            }
        }

        void remove() {
            if (parent)
                parent->removeChild(shared_from_this());
        }

        virtual void addChild(std::shared_ptr<Node> child) {
            if (!child)
                return;
            child->remove();
            children.push_back(child);
            child->parent = this;
            if (isInScene)
                child->processEvent(AddToScene{});
        }

        virtual void removeChild(std::shared_ptr<Node> node) {
            if (!node) return;
            if (node->parent == this) {
                auto it = std::find(children.begin(), children.end(), node);
                if (it != children.end()) {
                    node->parent = nullptr;
                    children.erase(it);
                    node->processEvent(RemoveFromScene{});
                }
            }
        }
    };

}
