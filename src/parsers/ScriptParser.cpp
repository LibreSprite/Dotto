// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <fs/FileSystem.hpp>
#include <log/Log.hpp>
#include <script/Engine.hpp>

using namespace fs;

class ScriptParser : public Parser {
public:
    bool canCache() override {return false;}

    Value parseFile(std::shared_ptr<File> file) override {
        script::Engine::PushDefault oldEngine;

        auto type = file->type();
        script::Engine::setDefault(type, {type});
        std::shared_ptr<script::Engine> engine = inject<script::Engine>{};
        if (!engine) {
            logE("No engine for ", type, " scripts");
            return nullptr;
        }

        engine->scriptName = file->getUID();

        if (!engine->eval(file->readTextFile())) {
            logE("Error parsing script");
            return nullptr;
        }
        engine->raiseEvent({"init"});
        return engine;
    }
};

static Parser::Shared<ScriptParser> js{"js"};
static Parser::Shared<ScriptParser> duk{"duk"};

#ifdef SCRIPT_ENGINE_LUA
static Parser::Shared<ScriptParser> lua{"lua"};
#endif
