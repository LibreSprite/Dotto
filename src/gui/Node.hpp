// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>
#include <common/Rect.hpp>
#include <gui/EventHandler.hpp>
#include <gui/Unit.hpp>

class Flow;
class Graphics;

namespace ui {

    class Controller;

    class Node : public Injectable<Node>,
                 public EventHandler,
                 public Model,
                 public std::enable_shared_from_this<Node> {
        friend class EventHandler;
        Vector<std::shared_ptr<Node>> children;
        Node* parent = nullptr;
        bool isInScene = false;
        bool isDirty = true;
        std::shared_ptr<Flow> flowInstance;
        HashMap<String, std::shared_ptr<Controller>> controllers;

        void reflow();
        void reattach();

    protected:
        void forwardToChildren(const Event& event);
        virtual void eventHandler(const AddToScene& event);

        virtual void eventHandler(const RemoveFromScene& event) {
            isInScene = false;
        }

        virtual void eventHandler(const Focus& event) {
            parent->processEvent(FocusChild{this});
        }

        virtual void eventHandler(const Blur& event) {
            parent->processEvent(BlurChild{this});
        }

    public:
        Rect localRect, globalRect;

        Property<String> id{this, "id", ""};
        Property<String> controllerName{this, "controller", "", &Node::reattach};
        Property<bool> visible{this, "visible", true};
        Property<bool> inputEnabled{this, "inputEnabled", true};
        Property<bool> debug{this, "debug"};

        Property<bool> hideOverflow{this, "overflow-hidden", false};
        Property<bool> absolute{this, "absolute", false, &Node::resize};
        Property<Unit> x{this, "x", {"0px"}, &Node::resize};
        Property<Unit> y{this, "y", {"0px"}, &Node::resize};

        Property<Unit> width{this, "width", {""}, &Node::resize};
        Property<Unit> minWidth{this, "min-width", {"10px"}, &Node::resize};
        Property<Unit> maxWidth{this, "max-width", {"100%"}, &Node::resize};

        Property<Unit> height{this, "height", {""}, &Node::resize};
        Property<Unit> minHeight{this, "min-height", {"10px"}, &Node::resize};
        Property<Unit> maxHeight{this, "max-height", {"100%"}, &Node::resize};

        Property<Rect> padding{this, "padding", Rect{}, &Node::resize};

        Property<String> flow{this, "flow", "column", &Node::reflow};
        Property<S32> zIndex{this, "z", 0};
        Property<String> forward{this, "forward"};

        Node();
        ~Node();

        static std::shared_ptr<Node> fromXML(const String& widgetName);

        virtual bool init(const PropertySet& properties);

        void load(const PropertySet& set) override;

        using Model::set;

        void set(const String& key, Value& value, bool debug = false) override;

        std::shared_ptr<Node> findChildById(const String& targetId);

        U32 getChildSeparation(std::shared_ptr<Node> child);

        void bringToFront(std::shared_ptr<Node> child);

        void processEvent(const Event& event) override;

        virtual void setDirty();

        const Vector<std::shared_ptr<Node>>& getChildren() {
            return children;
        }

        virtual bool update();

        virtual void focus(std::shared_ptr<ui::Node> child = nullptr) {
            if (parent) {
                parent->focus(child ?: shared_from_this());
            }
        }

        virtual void blur(std::shared_ptr<ui::Node> child = nullptr) {
            if (parent) {
                parent->blur(child ?: shared_from_this());
            }
        }

        Node* getParent() const {
            return parent;
        }

        virtual void resize() {
            if (parent) {
                parent->resize();
            }
        }

        virtual void doResize();

        virtual void onResize() {}

        virtual void draw(S32 z, Graphics& gfx);

        void remove() {
            if (parent)
                parent->removeChild(shared_from_this());
        }

        void removeAllChildren() {
            while (!children.empty()) {
                removeChild(children.back());
            }
        }

        virtual void addChild(std::shared_ptr<Node> child);

        virtual void removeChild(std::shared_ptr<Node> node);
    };

}
