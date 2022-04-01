// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Config.hpp>
#include <script/api/ModelScriptObject.hpp>
#include <script/Value.hpp>

class ConfigScriptObject : public script::ScriptObject {
public:
    inject<Config> config;

    void postInject() override {
        addFunction("get", [=](const String& name) -> script::Value {
            auto& map = config->properties->getMap();
            auto value = map.find(name);
            if (value == map.end() || !value->second)
                return nullptr;
            return getEngine().toValue(*value->second);
        });

        makeGlobal("config");
    }
};

static script::ScriptObject::Shared<ConfigScriptObject> nsoType{"ConfigScriptObject", {"global"}};
