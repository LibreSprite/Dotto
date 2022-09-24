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
            std::shared_ptr<PropertySet> properties = config->properties;
            auto parts = split(name, ".");
            for (std::size_t i = 0, max = parts.size(); i < max; ++i) {
                if (!properties)
                    break;
                auto& map = properties->getMap();
                auto value = map.find(parts[i]);
                if (value == map.end() || !value->second)
                    return nullptr;
                if (i == max - 1)
                    return getEngine().toValue(*value->second);
                properties = value->second->get<std::shared_ptr<PropertySet>>();
            }
            return nullptr;
        });

        addFunction("set", [=](const String& name, const script::Value& value) {
            std::shared_ptr<PropertySet> properties = config->properties;
            auto parts = split(name, ".");
            for (std::size_t i = 0, max = parts.size(); i < max; ++i) {
                if (!properties)
                    break;
                if (i == max - 1) {
                    properties->set(parts[i], value.get());
                    config->dirty();
                    logI("Set ", parts[i], value);
                    return true;
                }
                auto& map = properties->getMap();
                auto value = map.find(parts[i]);
                if (value == map.end() || !value->second) {
                    auto child = std::make_shared<PropertySet>();
                    properties->set(parts[i], child);
                    properties = child;
                } else {
                    properties = value->second->get<std::shared_ptr<PropertySet>>();
                }
            }
            return false;
        });

        makeGlobal("config");
    }
};

static script::ScriptObject::Shared<ConfigScriptObject> nsoType{"ConfigScriptObject", {"global"}};
