// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/PropertySet.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class ModelScriptObject : public script::ScriptObject {
    std::weak_ptr<Model> weak;
public:
    Value getWrapped() override {
        return weak.lock();
    }

    void setWrapped(const Value& vmodel) override {
        script::ScriptObject::setWrapped(vmodel);
        std::shared_ptr<Model> model = vmodel;
        weak = model;

        addFunction("apply", [=](){
            if (auto model = weak.lock()) {
                auto& args = script::Function::varArgs();
                auto ps = &model->getPropertySet();
                if (!args.empty()) {
                    if (args[0].type == script::Value::Type::OBJECT && args[0].data.object_v) {
                        auto value = args[0].data.object_v->getWrapped();
                        if (value.has<std::shared_ptr<PropertySet>>()) {
                            ps = value.get<std::shared_ptr<PropertySet>>().get();
                        }
                    }
                }
                if (!ps) {
                    logI("Unexpected apply type: ", args[0].str());
                    return 0;
                }
                model->load(*ps);
            }
            return 0;
        });

        addFunction("set", [=](const String& name, const script::Value& value) {
            if (auto model = weak.lock()) {
                if (value.type == script::Value::Type::OBJECT && value.data.object_v) {
                    model->set(name, value.data.object_v->getWrapped());
                } else {
                    model->set(name, nullptr);
                }
            } else {
                logE("Model expired, could not set ", name, ".");
            }
            return value;
        });

        addFunction("get", [=](const String& name) -> script::Value {
            if (auto model = weak.lock()) {
                auto value = model->get(name);
                return getEngine().toValue(value ? *value : Value{nullptr});
            } else {
                logE("Model expired, could not get ", name, ".");
            }
            return nullptr;
        });

        for (auto& name : model->getPropertyNames()) {
            addProperty(name,
                        [=]()->script::Value {
                            if (auto model = weak.lock()) {
                                if (auto value = model->get(name))
                                    return getEngine().toValue(*value);
                            } else {
                                logE("Model expired, could not get ", name, ".");
                            }
                            return nullptr;
                        },
                        [=](const script::Value& value) {
                            if (auto model = weak.lock()) {
                                if (value.type == script::Value::Type::OBJECT && value.data.object_v) {
                                    model->set(name, value.data.object_v->getWrapped());
                                } else
                                    model->set(name, value.get());
                            } else {
                                logE("Model expired, could not get ", name, ".");
                            }
                            return value;
                        });
        }
    }
};
