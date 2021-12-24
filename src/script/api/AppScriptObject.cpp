// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <sstream>

#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class AppScriptObject : public script::ScriptObject {
public:
    script::ScriptObject* target;

    AppScriptObject() {
        inject<script::ScriptTarget> scriptTarget;
        target = getEngine().getScriptObject(scriptTarget->target, scriptTarget->wrapperName);
        addProperty("target", [this]{return target;});
        makeGlobal("app");
    }

};

static script::ScriptObject::Regular<AppScriptObject> reg("AppScriptObject", {"global"});
