// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>
#include <log/Log.hpp>
#include <script/ScriptObject.hpp>
#include <script/Value.hpp>

namespace script {
    class Engine : public Injectable<Engine>, public std::enable_shared_from_this<Engine> {
    protected:
        inject<Log> log;

        void execAfterEval(bool success) {
            for (auto& listener : afterEvalListeners) {
                listener(success);
            }
            afterEvalListeners.clear();
            for (auto it = objectMap.begin(); it != objectMap.end();) {
                auto& pair = it->second;
                if (pair.object.expired()) {
                    it = objectMap.erase(it);
                } else {
                    ++it;
                }
            }
            autoHoldList.clear();
        }

        String internalScriptObjectName;

        Engine(const String& internalScriptObjectName) : internalScriptObjectName{internalScriptObjectName} {}

    public:
        String scriptName;

        void initGlobals() {
            if (globalScriptObjects.empty())
                globalScriptObjects = ScriptObject::getAllWithFlag("global");
        }

        script::Value toValue(const ::Value& value) {
            script::Value out;
            if (!out.set(value))
                out = getScriptObject(value, value.typeName());
            return out;
        }

        ScriptObject* getScriptObject(const ::Value& value, const String& injectionName) {
            auto object = value.getShared();
            if (!object)
                return nullptr;
            autoHoldList.insert(object);
            auto it = objectMap.find(object.get());
            if (it != objectMap.end())
                return it->second.wrapper.get();

            PushDefault engine{this};
            InternalScriptObject::PushDefault iso{engine ? internalScriptObjectName : ""};

            inject<ScriptObject> wrapper{injectionName};
            autoHoldList.insert(wrapper);
            wrapper->setWrapped(value);
            objectMap[object.get()] = {object, wrapper};
            return wrapper.get();
        }

        void hold(std::shared_ptr<ScriptObject> obj) {
            obj->hold();
            holdList.insert(obj);
        }

        void release(std::shared_ptr<ScriptObject> obj) {
            auto it = holdList.find(obj);
            if (it != holdList.end()) {
                autoHoldList.insert(obj);
                holdList.erase(it);
                obj->release();
            } else {
                logI("Missing ", obj);
            }
        }

        virtual bool eval(const std::string& code) = 0;
        virtual bool raiseEvent(const Vector<String>& event) = 0;

        void afterEval(std::function<void(bool)>&& callback) {
            afterEvalListeners.emplace_back(std::move(callback));
        }

        ScriptObject* getGlobal(const String& name) {
            auto it = globalScriptObjectIndex.find(name);
            return it == globalScriptObjectIndex.end() ? nullptr : it->second;
        }

    private:
        friend class InternalScriptObject;

        Vector<inject<ScriptObject>> globalScriptObjects;
        HashMap<String, ScriptObject*> globalScriptObjectIndex;
        Vector<std::function<void(bool)>> afterEvalListeners;

        struct WrapperPair {
            std::weak_ptr<void> object;
            std::shared_ptr<ScriptObject> wrapper;
        };
        HashMap<void*, WrapperPair> objectMap;
        HashSet<std::shared_ptr<void>> autoHoldList;
        HashSet<std::shared_ptr<void>> holdList;
    };
}
