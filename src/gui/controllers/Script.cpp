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
    Property<String> script{this, "script", "", &ScriptController::loadScript};
    PubSub<msg::Flush> pub{this};

    void loadScript() {
        engine.reset();
        if (!script->empty()) {
            script::ScriptTarget target{node()->shared_from_this(), "NodeScriptObject"};
            engine = FileSystem::parse(script);
        }
    }

    void on(msg::Flush& flush) {
        flush.hold(engine);
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
static script::ScriptObject::Shared<ModelScriptObject> mso{"ModelScriptObject"};
