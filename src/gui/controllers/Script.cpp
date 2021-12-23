// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "FileSystem.hpp"
#include "ScriptObject.hpp"
#include "types.hpp"
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class ScriptController : public ui::Controller {
public:
    std::shared_ptr<script::Engine> engine;
    String current;
    Property<String> script{this, "script", "", &ScriptController::loadScript};

    void loadScript() {
        if (*script == current)
            return;
        current = script;
        engine.reset();
        if (!script->empty())
            engine = FileSystem::parse(current);
    }

    void attach() override {
        logI("Script attached ", current);
    }

    void eventHandler(const ui::KeyDown& event) {
    }

    void eventHandler(const ui::KeyUp& event) {
    }
};

static ui::Controller::Shared<ScriptController> scriptController{"script"};
