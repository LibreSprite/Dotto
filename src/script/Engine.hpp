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
        }

    public:
        void initGlobals() {
            if (scriptObjects.empty())
                scriptObjects = ScriptObject::getAllWithFlag("global");
        }

        virtual bool eval(const std::string& code) = 0;
        virtual bool raiseEvent(const std::string& event) = 0;

        void afterEval(std::function<void(bool)>&& callback) {
            afterEvalListeners.emplace_back(std::move(callback));
        }

    private:
        Provides provides{this};
        Vector<inject<ScriptObject>> scriptObjects;
        Vector<std::function<void(bool)>> afterEvalListeners;
    };
}
