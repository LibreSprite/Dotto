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
            holdList.clear();
        }

    public:
        void initGlobals() {
            if (globalScriptObjects.empty())
                globalScriptObjects = ScriptObject::getAllWithFlag("global");
        }

        ScriptObject* getScriptObject(std::shared_ptr<void> object, const String& injectionName) {
            if (!object)
                return nullptr;
            holdList.insert(object);
            auto it = objectMap.find(object.get());
            if (it != objectMap.end())
                return it->second.wrapper.get();
            inject<ScriptObject> wrapper{injectionName};
            wrapper->setWrapped(object);
            objectMap[object.get()] = {object, wrapper};
            return wrapper.get();
        }

        virtual bool eval(const std::string& code) = 0;
        virtual bool raiseEvent(const std::string& event) = 0;

        void afterEval(std::function<void(bool)>&& callback) {
            afterEvalListeners.emplace_back(std::move(callback));
        }

    private:
        Provides provides{this};
        Vector<inject<ScriptObject>> globalScriptObjects;
        Vector<std::function<void(bool)>> afterEvalListeners;

        struct WrapperPair {
            std::weak_ptr<void> object;
            std::shared_ptr<ScriptObject> wrapper;
        };
        HashMap<void*, WrapperPair> objectMap;
        HashSet<std::shared_ptr<void>> holdList;
    };
}
