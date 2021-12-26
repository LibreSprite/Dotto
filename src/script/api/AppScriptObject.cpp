// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <sstream>

#include <cmd/Command.hpp>
#include <common/PropertySet.hpp>
#include <script/Engine.hpp>
#include <script/Function.hpp>
#include <script/ScriptObject.hpp>

class AppScriptObject : public script::ScriptObject {
public:
    script::Value target;

    AppScriptObject() {
        target = getEngine().toValue(inject<script::ScriptTarget>{}->target);
        addProperty("target", [this]{return target;});
        addFunction("command", [](const String& name){
            inject<Command> command{name};
            if (!command)
                return false;
            PropertySet properties;
            auto& args = script::Function::varArgs();
            for (std::size_t i = 1, size = args.size(); i + 1 < size; i += 2) {
                properties.set(args[i], args[i + 1].str());
            }
            command->load(properties);
            command->run();
            return true;
        });
        makeGlobal("app");
    }

};

static script::ScriptObject::Regular<AppScriptObject> reg("AppScriptObject", {"global"});
