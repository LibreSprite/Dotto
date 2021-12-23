// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <sstream>

#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class ConsoleScriptObject : public script::ScriptObject {
public:
    inject<Log> logger;

    ConsoleScriptObject() {
        addMethod("log", this, &ConsoleScriptObject::log);
        addMethod("assert", this, &ConsoleScriptObject::_assert);
        makeGlobal("console");
    }

    void _assert(bool condition, const std::string& msg){
        if (!condition)
            log();
    }

    void log() {
        std::stringstream stream;
        bool first = true;
        for (auto& arg : script::Function::varArgs()) {
            if (!first) {
                stream << " ";
            }
            first = false;
            stream << arg.str();
        }
        logger->write(Log::Level::INFO, stream.str());
    }
};

static script::ScriptObject::Regular<ConsoleScriptObject> reg("ConsoleScriptObject", {"global"});
