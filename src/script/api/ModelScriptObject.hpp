// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/PropertySet.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class ModelScriptObject : public script::ScriptObject {
    std::weak_ptr<Model> model;
public:
    virtual void* getWrapped(){
        return model.lock().get();
    }

    void setWrapped(std::shared_ptr<void> vmodel) {
        script::ScriptObject::setWrapped(vmodel);
        auto model = std::static_pointer_cast<Model>(vmodel);
        this->model = model;
        for (auto& entry : model->getPropertySet().getMap()) {
            auto& name = entry.first;
            auto& ptr = entry.second;
            addProperty(name,
                        [=]{return getEngine().toValue(*ptr);},
                        [=](const script::Value& value) {
                            if (auto model = this->model.lock()) {
                                model->set(name, value.get());
                            }
                            return value;
                        });
        }
    }
};
