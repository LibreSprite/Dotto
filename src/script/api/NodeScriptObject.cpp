// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <script/api/AppScriptObject.hpp>
#include <script/api/ModelScriptObject.hpp>
#include <script/Engine.hpp>
#include <script/Value.hpp>

class NodeScriptObject : public ModelScriptObject {
    std::weak_ptr<ui::Node> weak;

public:
    ~NodeScriptObject() {
        if (auto node = weak.lock()) {
            node->removeEventListeners(this);
        }
    }

    Value getWrapped() override {
        return weak.lock();
    }

    void setWrapped(const Value& vmodel) override {
        std::shared_ptr<ui::Node> node = vmodel;
        ModelScriptObject::setWrapped(std::static_pointer_cast<Model>(node));
        auto weak = std::weak_ptr(node);
        this->weak = weak;

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

        addProperty("parent", [=]() -> script::Value {
            if (auto node = weak.lock()) {
                if (auto parent = node->getParent())
                    return getEngine().toValue(parent->shared_from_this());
            }
            return nullptr;
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

        addFunction("focus", [=]() {
            if (auto node = weak.lock())
                node->focus();
            return 0;
        });

        addFunction("blur", [=]() {
            if (auto node = weak.lock())
                node->blur();
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

        addFunction("getChildSeparation", [=](script::ScriptObject* obj) -> U32 {
            if (!obj) return 0;
            if (auto node = weak.lock()) {
                auto child = dynamic_cast<ui::Node*>(static_cast<Model*>(obj->getWrapped()));
                if (!child)
                    return 0;
                return node->getChildSeparation(child->shared_from_this());
            }
            return 0;
        });

        addFunction("createChild", [=](const String& name) -> script::Value {
            if (auto node = weak.lock()) {
                auto child = ui::Node::fromXML(name);
                if (child) {
                    node->addChild(child);
                    return getEngine().toValue(child);
                }
            }
            return nullptr;
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
                auto it = eventBinders.find(trim(tolower(name)));
                if (it == eventBinders.end())
                    return 0;
                it->second(node.get(), name);
                eventBinders.erase(it);
                return 1;
            }
            return 0;
        });

        createEventBinder<ui::AddToScene>("addtoscene");
        createEventBinder<ui::Blur>("blur");
        createEventBinder<ui::Focus>("focus");
        createEventBinder<ui::RemoveFromScene>("removefromscene");
        createEventBinder<ui::MouseEnter>("mouseenter");
        createEventBinder<ui::MouseLeave>("mouseleave");
        createEventBinder<ui::MouseMove>("mousemove");
        createEventBinder<ui::MouseUp>("mouseup");
        createEventBinder<ui::MouseDown>("mousedown");
        createEventBinder<ui::Click>("click");
        createEventBinder<ui::KeyDown>("keydown");
        createEventBinder<ui::KeyUp>("keyup");
        createEventBinder<ui::Drag>("drag");
        createEventBinder<ui::Drop>("drop");
    }

    HashMap<String, std::function<void(ui::Node*, const String&)>> eventBinders;

    template <typename Type>
    void createEventBinder(const String& name) {
        eventBinders[name] = [=](ui::Node* node, const String& name) {
            auto weakapp = getEngine().getGlobal("app")->weak_from_this();
            auto handler = [=](const auto& event) {
                if (auto app = std::static_pointer_cast<AppScriptObject>(weakapp.lock())) {
                    auto target = std::static_pointer_cast<script::ScriptObject>(shared_from_this());
                    app->setTarget(target);
                    if (event.target == node) {
                        app->setEventTarget(target);
                    } else if (event.target) {
                        app->setEventTarget(event.target->shared_from_this());
                    } else {
                        app->setEventTarget(nullptr);
                    }
                    getEngine().raiseEvent(event.toStrings(name));
                }
            };
            node->addEventListener<Type>(this, handler);
        };
    }
};

static script::ScriptObject::Shared<NodeScriptObject> nsoType{typeid(std::shared_ptr<ui::Node>).name()};
