// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <sstream>
#include <optional>

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/PropertySet.hpp>
#include <common/Value.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Selection.hpp>
#include <fs/FileDialog.hpp>
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
    script::Value eventTarget;
    std::optional<PubSub<msg::Tick>> tick;
    PubSub<> pub{this};

    void setTarget(const ::Value& target) override {
        this->target = getEngine().toValue(target);
    }

    void setEventTarget(const Value& eventTarget) override {
        this->eventTarget = getEngine().toValue(eventTarget);
    }

    AppScriptObjectImpl() {
        if (auto scripttarget = inject<script::ScriptTarget>{InjectSilent::Yes}) {
            setTarget(scripttarget->target);
        }

        addProperty("target", [this]{return target;});

        addProperty("eventTarget", [this]{return eventTarget;});

        addProperty("activeCell", [this]{
            return getEngine().toValue(inject<Cell>{"activecell"}.shared());
        });

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

        addFunction("write", [=](const String& path) {
            auto& args = script::Function::varArgs();
            if (args.size() < 2)
                return false;

            auto& argv = args[1];
            Value value;
            if (argv.type == script::Value::Type::OBJECT)
                value = argv.data.object_v->getWrapped();
            else
                value = argv.get();

            if (!FileSystem::write(path, value)) {
                logE("Could not save ", value.toString(), " to ", path);
                return false;
            }

            return true;
        });

        addFunction("open", [=](const String& filters, const String& title="Script", const String& description = ""){
            inject<FileDialog> dialog;
            dialog->filterDescription = description;
            dialog->title = title;
            dialog->filters = split(filters, "|");
            dialog->open();
            return join(dialog->result, "|");
        });

        addFunction("save", [=](const String& filters, const String& title="Script", const String& description = ""){
            inject<FileDialog> dialog;
            dialog->filterDescription = description;
            dialog->title = title;
            dialog->filters = split(filters, "|");
            dialog->save();
            return join(dialog->result, "|");
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

        addFunction("quit", [=](){
            pub(msg::RequestShutdown{});
            return true;
        });

        makeGlobal("app");
    }

    void on(msg::Tick&) {getEngine().raiseEvent({"tick"});}
};

static script::ScriptObject::Shared<AppScriptObjectImpl> reg("AppScriptObject", {"global"});
