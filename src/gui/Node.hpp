// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>
#include <gui/Events.hpp>
#include <gui/EventHandler.hpp>
#include <gui/Rect.hpp>
#include <gui/Unit.hpp>
#include <log/Log.hpp>

namespace ui {

    class Node : public Injectable<Node>,
                 public EventHandler,
                 public Serializable,
                 public std::enable_shared_from_this<Node> {
        friend class EventHandler;
        Vector<std::shared_ptr<Node>> children;
        Node* parent = nullptr;
        bool isInScene = false;
        bool isDirty = true;

    protected:
        void forwardToChildren(const Event& event) {
            for (auto& child : children)
                child->processEvent(event);
        }

        virtual void eventHandler(const AddToScene& event) {
            isInScene = true;
            if (isDirty && parent) // parent is null for root node
                parent->setDirty();
        }

        virtual void eventHandler(const RemoveFromScene& event) {
            isInScene = false;
        }

    public:
        Property<Unit> width{this, "width"};
        Property<Unit> height{this, "height"};
        Rect localRect, globalRect;

        Node() {
            addEventListener<AddToScene, RemoveFromScene>(this);
        }

        virtual bool init(const PropertySet& properties) {
            load(properties);
            return true;
        }

        virtual void processEvent(const Event& event) {
            if (event.bubble == Event::Bubble::Down && !event.cancel)
                forwardToChildren(event);
            EventHandler::processEvent(event);
            if (event.bubble == Event::Bubble::Up && !event.cancel && parent)
                parent->processEvent(event);
        }

        void setDirty() {
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

        void remove() {
            if (parent)
                parent->removeChild(shared_from_this());
        }

        void addChild(std::shared_ptr<Node> child) {
            if (!child)
                return;
            child->remove();
            children.push_back(child);
            child->parent = this;
            if (isInScene)
                child->processEvent(AddToScene{});
        }

        void removeChild(std::shared_ptr<Node> node) {
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
