// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/line.hpp>
#include <doc/Surface.hpp>
#include <memory>
#include <script/ScriptObject.hpp>
#include <tools/Tool.hpp>
#include <script/api/AppScriptObject.hpp>
#include <script/Engine.hpp>

class ToolScriptObject : public script::ScriptObject {
public:
    const Vector<Point>* points;
    std::shared_ptr<script::ScriptObject> surface;

    ToolScriptObject() {
        surface = inject<script::ScriptObject>{typeid(std::shared_ptr<Surface>).name()};
        addProperty("count", [=]{return 5;});
        addProperty("color", [=]{return Tool::color.toU32();});
        addProperty("surface", [=]{return surface.get();});
        addProperty("lastX", [=]{return points->back().x;});
        addProperty("lastY", [=]{return points->back().y;});
        addFunction("x", [=](U32 id) {
            return id < points->size() ? points->at(id).x : 0;
        });
        addFunction("y", [=](U32 id) {
            return id < points->size() ? points->at(id).y : 0;
        });
    }
};

class Script : public Tool {
public:
    std::shared_ptr<script::Engine> engine;
    std::shared_ptr<AppScriptObject> app;
    std::shared_ptr<ToolScriptObject> tso = std::make_shared<ToolScriptObject>();

    Script() :
        engine{inject<script::Engine>{"toolengine"}},
        app{inject<script::ScriptObject>{"toolapp"}.shared<AppScriptObject>()}
        {
    }

    virtual void begin(Surface* surface, const Vector<Point>& points) {
        tso->surface->setWrapped(surface->shared_from_this());
        tso->points = &points;
        app->setTarget(std::static_pointer_cast<script::ScriptObject>(tso));
        engine->raiseEvent({"toolstart"});
    }

    virtual void update(Surface* surface, const Vector<Point>& points) {
        tso->surface->setWrapped(surface->shared_from_this());
        tso->points = &points;
        app->setTarget(std::static_pointer_cast<script::ScriptObject>(tso));
        engine->raiseEvent({"toolupdate"});
    }

    virtual void end(Surface* surface, const Vector<Point>& points) {
        tso->surface->setWrapped(surface->shared_from_this());
        tso->points = &points;
        app->setTarget(std::static_pointer_cast<script::ScriptObject>(tso));
        engine->raiseEvent({"toolend"});
    }
};

static Tool::Shared<Script> reg{"script", {"noauto"}};
