// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <regex>

#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>
#include <task/TaskManager.hpp>

class ShellScriptObject : public script::ScriptObject {
public:
    TaskHandle handle;

    enum class State {
        Ready,
        Busy,
        Error
    } state = State::Ready;

    U32 code = 0;
    String stdout;
    std::shared_ptr<script::EngineObjRef> event;
    std::shared_ptr<script::Engine> engine;

    ShellScriptObject() {
        addMethod("exec", &ShellScriptObject::exec);
        addProperty("state", [=]{return (int) state;});
        addProperty("code", [=]{return code;});
        addProperty("stdout", [=]{return stdout;});
        makeGlobal("shell");
    }

    String getShellCmd() {
        auto& args = script::Function::varArgs();
        String out;
        String clean;
        event.reset();
        for (std::size_t i = 0, max = args.size(); i < max; ++i) {
            if (args[i].type == script::Value::Type::ENGOBJREF) {
                event = args[i];
                continue;
            }
            String arg = args[i];
            if (arg.empty())
                continue;

            if (!out.empty())
                out.push_back(' ');
            clean.clear();
            clean.push_back('\"');

            for (auto i : arg) {
                if (i == '\"')
                    clean += "\\\"";
                else
                    clean.push_back(i);
            }

            clean.push_back('\"');
            out += clean;
        }
        return out;
    }

    script::Value exec() {
        auto cmd = getShellCmd();
        if (event) {
            async(cmd);
            return event;
        }

        script::Value ret;
        String result;
        auto onClose = [&](FILE* file){
            int status = pclose(file);
            if (status != 0) {
                ret = status;
            } else {
                ret = result;
            }
        };
        char buffer[1024];
        {
            std::unique_ptr<FILE, decltype(onClose)> pipe(popen(cmd.c_str(), "r"), onClose);
            while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
                result += buffer;
            }
        }
        stdout = std::move(result);
        return ret;
    }

    void async(const String& cmd){
        engine = getEngine().shared_from_this();

        if (state == State::Busy) {
            logE("Busy, can't request for [", cmd, "]");
            return;
        }

        state = State::Busy;
        std::weak_ptr<ShellScriptObject> weak{std::static_pointer_cast<ShellScriptObject>(shared_from_this())};
        handle = inject<TaskManager>{}->add(
            [=]()->std::pair<String, int>{
                try {
                    String result;
                    int code;
                    auto onClose = [&](FILE* file){
                        code = pclose(file);
                    };
                    char buffer[1024];
                    {
                        std::unique_ptr<FILE, decltype(onClose)> pipe(popen(cmd.c_str(), "r"), onClose);
                        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
                            result += buffer;
                        }
                    }
                    return {result, code};
                } catch (const std::exception& ex) {
                    return {ex.what(), -1};
                }
            },
            [=](std::pair<String, int>&& resp){
                auto that = weak.lock();
                if (!that)
                    return;
                that->stdout = std::move(resp.first);
                that->code = resp.second;
                that->state = State::Ready;
                event->call({that->code, that->stdout});
                event.reset();
                auto copy = engine;
                engine.reset();
            });
    }
};

static script::ScriptObject::Shared<ShellScriptObject> reg("ShellScriptObject", {"global"});
