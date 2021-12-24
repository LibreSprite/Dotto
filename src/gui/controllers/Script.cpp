// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/PropertySet.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>
#include <script/ScriptObject.hpp>

class ModelScriptObject : public script::ScriptObject {
    Model* model;
public:
    void setWrapped(void* vmodel) {
        model = static_cast<Model*>(vmodel);
        for (auto& entry : model->getPropertySet().getMap()) {
            auto& name = entry.first;
            auto& ptr = entry.second;
            addProperty(name,
                        [=]{return script::Value{}.set(*ptr);},
                        [=](const script::Value& value){
                            model->set(name, value.get());
                            return value;
                        });
        }
    }
};

static script::ScriptObject::Shared<ModelScriptObject> mso{"ModelScriptObject"};

class ScriptController : public ui::Controller {
public:
    std::shared_ptr<script::Engine> engine;
    Property<String> script{this, "script", "", &ScriptController::loadScript};

    void loadScript() {
        engine.reset();
        if (!script->empty()) {
            script::ScriptTarget target{static_cast<Model*>(node())};
            engine = FileSystem::parse(script);
        }
    }

    void attach() override {
        logI("Script attached ", script);
    }

    void eventHandler(const ui::KeyDown& event) {
    }

    void eventHandler(const ui::KeyUp& event) {
    }
};

static ui::Controller::Shared<ScriptController> scriptController{"script"};
