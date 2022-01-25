// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "log/Log.hpp"
#include "script/Function.hpp"
#include <common/PropertySet.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class PropertySetScriptObject : public script::ScriptObject {
    std::weak_ptr<PropertySet> weak;
public:
    Value getWrapped() override {
        return weak.lock();
    }

    void setWrapped(const Value& vmodel) override {
        logI("Setting wrapped ", vmodel.typeName());
        script::ScriptObject::setWrapped(vmodel);
        std::shared_ptr<PropertySet> model = vmodel;
        weak = model;
    }

    PropertySetScriptObject() {
        addFunction("apply", [=](const script::Value& value){
            if (auto model = weak.lock()) {
                if (value.type == script::Value::Type::OBJECT && value.data.object_v) {
                    auto object = value.data.object_v->getWrapped();
                    if (object.has<std::shared_ptr<PropertySet>>()) {
                        auto ps = object.get<std::shared_ptr<PropertySet>>();
                        model->append(*ps);
                        return true;
                    }
                }
                logI("Unexpected apply type: ", script::Function::varArgs()[0].str());
            }
            return false;
        });

        addFunction("set", [=](const String& name, const script::Value& value) {
            if (auto model = weak.lock()) {
                if (value.type == script::Value::Type::OBJECT && value.data.object_v) {
                    model->set(name, value.data.object_v->getWrapped());
                } else
                    model->set(name, nullptr);
            } else {
                logE("PropertySet expired, could not set ", name, ".");
            }
            return value;
        });

        addFunction("get", [=](const String& name) -> script::Value {
            if (auto model = weak.lock()) {
                auto it = model->getMap().find(name);
                if (it != model->getMap().end()) {
                    auto value = it->second;
                    return getEngine().toValue(value ? *value : Value{nullptr});
                }
            } else {
                logE("PropertySet expired, could not get ", name, ".");
            }
            return nullptr;
        });
    }
};

static script::ScriptObject::Shared<PropertySetScriptObject> msoType{typeid(std::shared_ptr<PropertySet>).name()};
