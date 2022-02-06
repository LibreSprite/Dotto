// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

namespace script {

    InternalScriptObject::InternalScriptObject() {
        if(auto liveInstances = getLiveInstances())
            liveInstances->insert(this);
    }

    InternalScriptObject::~InternalScriptObject() {
        if(auto liveInstances = getLiveInstances())
            liveInstances->erase(this);
    }

    void InternalScriptObject::makeGlobal(const String& name) {
        engine->globalScriptObjectIndex[name] = scriptObject;
    }
}
