// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Node.hpp>
#include <script/api/ModelScriptObject.hpp>
#include <script/Engine.hpp>

class NodeScriptObject : public ModelScriptObject {
public:
    void setWrapped(std::shared_ptr<void> vmodel) {
        auto sharednode = std::static_pointer_cast<ui::Node>(vmodel);
        auto weak = std::weak_ptr(sharednode);
        ModelScriptObject::setWrapped(std::static_pointer_cast<Model>(sharednode));

        addFunction("findChildById", [=](const String& id){
            script::ScriptObject* so = nullptr;
            if (auto node = weak.lock()) {
                auto child = node->findChildById(id);
                so = getEngine().getScriptObject(child, "NodeScriptObject");
            }
            return so;
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
    }
};

static script::ScriptObject::Shared<NodeScriptObject> nso{"NodeScriptObject"};
