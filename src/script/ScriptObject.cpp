// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

namespace script {

    InternalScriptObject::InternalScriptObject() {
        liveInstances.insert(this);
    }

    InternalScriptObject::~InternalScriptObject() {
        liveInstances.erase(this);
    }
}
