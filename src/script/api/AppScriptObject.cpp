// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <sstream>

#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class AppScriptObject : public script::ScriptObject {
public:
    script::Value target;

    AppScriptObject() {
        target = getEngine().toValue(inject<script::ScriptTarget>{}->target);
        addProperty("target", [this]{return target;});
        makeGlobal("app");
    }

};

static script::ScriptObject::Regular<AppScriptObject> reg("AppScriptObject", {"global"});
