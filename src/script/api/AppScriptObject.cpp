// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <sstream>
#include <optional>

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/PropertySet.hpp>
#include <doc/Document.hpp>
#include <doc/Selection.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Node.hpp>
#include <script/api/AppScriptObject.hpp>
#include <script/Engine.hpp>
#include <script/Function.hpp>
#include <script/ScriptObject.hpp>
#include <tools/Tool.hpp>

class AppScriptObjectImpl : public AppScriptObject {
public:
    script::Value target;
    std::optional<PubSub<msg::Tick>> tick;

    void setTarget(const ::Value& target) override {
        this->target = getEngine().toValue(target);
    }

    AppScriptObjectImpl() {
        if (auto scripttarget = inject<script::ScriptTarget>{InjectSilent::Yes}) {
            setTarget(scripttarget->target);
        }

        addProperty("target", [this]{return target;});

        addProperty("window", [this]{
            auto node = PubSub<>::pub(msg::PollActiveWindow{}).node;
            return node ? getEngine().toValue(node->shared_from_this()) : nullptr;
        });

        addFunction("command", [this](const String& name){
            inject<Command> command{tolower(name)};
            if (!command)
                return false;
            PropertySet properties;
            auto& args = script::Function::varArgs();
            for (std::size_t i = 1, size = args.size(); i + 1 < size; i += 2) {
                auto& argv = args[i + 1];
                if (argv.type == script::Value::Type::OBJECT)
                    properties.set(args[i], argv.data.object_v->getWrapped());
                else
                    properties.set(args[i], argv.get());
            }
            command->load(properties);
            command->run();
            return true;
        });

        addFunction("addTool", [=](const String& name) {
            script::Engine::Provides engine{getEngine().shared_from_this(), "toolengine"};
            script::ScriptObject::Provides app{shared_from_this(), "toolapp"};
            inject<Tool>{"script"}->init(name);
            return true;
        });

        addFunction("addEventListener", [=](const String& name) {
            if (name == "tick") {
                if (!tick)
                    tick = this;
            }
            return true;
        });

        addFunction("parse", [=](const String& path) {
            return getEngine().toValue(FileSystem::parse(path));
        });

        addFunction("openWindow", [=](const String& name) -> script::Value {
            return ui::Node::fromXML(name) != nullptr;
        });

        addFunction("newSelection", [=]() -> script::Value {
            return getEngine().toValue(inject<Selection>{"new"}.shared());
        });

        addFunction("hold", [=](ScriptObject* obj) {
            if (obj)
                getEngine().hold(obj->shared_from_this());
            else
                logE("Script requesting null hold");
            return nullptr;
        });

        addFunction("release", [=](ScriptObject* obj) {
            if (obj)
                getEngine().release(obj->shared_from_this());
            return nullptr;
        });

        makeGlobal("app");
    }

    void on(msg::Tick&) {getEngine().raiseEvent({"tick"});}
};

static script::ScriptObject::Shared<AppScriptObjectImpl> reg("AppScriptObject", {"global"});
