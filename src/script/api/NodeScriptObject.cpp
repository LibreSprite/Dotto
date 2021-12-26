// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Node.hpp>
#include <script/api/ModelScriptObject.hpp>
#include <script/Engine.hpp>

class NodeScriptObject : public ModelScriptObject {
    std::weak_ptr<ui::Node> weak;

public:
    ~NodeScriptObject() {
        if (auto node = weak.lock()) {
            node->removeEventListeners(this);
        }
    }

    void setWrapped(std::shared_ptr<void> vmodel) {
        auto sharednode = std::static_pointer_cast<ui::Node>(vmodel);
        auto weak = std::weak_ptr(sharednode);
        this->weak = weak;

        ModelScriptObject::setWrapped(std::static_pointer_cast<Model>(sharednode));

        addProperty("globalX", [=]()->script::Value {
            if (auto node = weak.lock()) return node->globalRect.x;
            return 0;
        });

        addProperty("globalY", [=]()->script::Value {
            if (auto node = weak.lock()) return node->globalRect.y;
            return 0;
        });

        addProperty("globalWidth", [=]()->script::Value {
            if (auto node = weak.lock()) return node->globalRect.width;
            return 0;
        });

        addProperty("globalHeight", [=]()->script::Value {
            if (auto node = weak.lock()) return node->globalRect.height;
            return 0;
        });

        addFunction("findChildById", [=](const String& id) -> script::Value {
            if (auto node = weak.lock()) {
                return getEngine().toValue(node->findChildById(id));
            }
            return nullptr;
        });

        addFunction("remove", [=]() {
            if (auto node = weak.lock())
                node->remove();
            return 0;
        });

        addFunction("removeChild", [=](script::ScriptObject* obj) {
            if (!obj) return 0;
            if (auto node = weak.lock()) {
                auto child = dynamic_cast<ui::Node*>(static_cast<Model*>(obj->getWrapped()));
                if (child)
                    node->removeChild(child->shared_from_this());
            }
            return 0;
        });

        addFunction("addChild", [=](script::ScriptObject* obj) {
            if (!obj) return 0;
            if (auto node = weak.lock()) {
                auto child = dynamic_cast<ui::Node*>(static_cast<Model*>(obj->getWrapped()));
                if (child) {
                    node->addChild(child->shared_from_this());
                }
            }
            return 0;
        });

        addFunction("addEventListener", [=](const String& name) {
            if (auto node = weak.lock()) {
                auto handler = [=](const auto& event) {
                    getEngine().raiseEvent(event.toStrings(name));
                };
                if (name == "mousemove")
                    node->addEventListener<ui::MouseMove>(this, handler);
                else if (name == "mouseup")
                    node->addEventListener<ui::MouseUp>(this, handler);
                else if (name == "mousedown")
                    node->addEventListener<ui::MouseDown>(this, handler);
            }
            return 0;
        });
    }
};

static script::ScriptObject::Shared<NodeScriptObject> nsoType{typeid(std::shared_ptr<ui::Node>).name()};
