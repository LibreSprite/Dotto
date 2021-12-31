// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <script/api/ModelScriptObject.hpp>
#include <tools/Tool.hpp>

class ScriptController : public ui::Controller {
public:
    std::shared_ptr<script::Engine> engine;
    Property<bool> run{this, "run", false};
    Property<String> script{this, "script", "", &ScriptController::loadScript};
    Property<String> type{this, "type", ""};
    Property<String> text{this, "text", "", &ScriptController::loadScript};

    void loadScript() {
        if (!node() || !*run)
            return;
        *run = false;

        if (!script->empty()) {
            script::ScriptTarget target{node()->shared_from_this()};
            engine = FileSystem::parse(script);
            return;
        }

        if (type->empty()) {
            engine.reset();
            return;
        }

        script::Engine::PushDefault oldEngine;
        script::Engine::setDefault(type, {type});
        script::ScriptTarget target{node()->shared_from_this()};
        engine = inject<script::Engine>{};
        if (engine)
            engine->eval(text);
    }

    void attach() override {
        *run = true;
        loadScript();
    }
};

static ui::Controller::Shared<ScriptController> scriptController{"script"};
static script::ScriptObject::Shared<ModelScriptObject> msoType{typeid(std::shared_ptr<script::ScriptObject>).name()};
