// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/line.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <script/ScriptObject.hpp>
#include <tools/Tool.hpp>
#include <script/api/AppScriptObject.hpp>
#include <script/api/ModelScriptObject.hpp>
#include <script/Engine.hpp>

class ToolScriptObject : public ModelScriptObject {
public:
    const Tool::Path* points;
    std::shared_ptr<script::ScriptObject> surface;
    U32 which = ~U32{};

    void postInject() override {
        surface = inject<script::ScriptObject>{typeid(std::shared_ptr<Surface>).name()};
        addProperty("which", [=]{return which;});
        addProperty("count", [=]{return U32(points->size());});
        addProperty("color", [=]{return Tool::color.toU32();});
        addProperty("surface", [=]{return surface.get();});
        addProperty("lastX", [=]{return points->back().x;});
        addProperty("lastY", [=]{return points->back().y;});
        addProperty("lastZ", [=]{return points->back().z;});
        addFunction("x", [=](U32 id) {
            return id < points->size() ? points->at(id).x : 0;
        });
        addFunction("y", [=](U32 id) {
            return id < points->size() ? points->at(id).y : 0;
        });
        addFunction("z", [=](U32 id) {
            return id < points->size() ? points->at(id).z : 0;
        });
    }
};

class Script : public Tool {
public:
    std::shared_ptr<script::Engine> engine;
    std::shared_ptr<AppScriptObject> app;
    std::shared_ptr<ToolScriptObject> tso = std::make_shared<ToolScriptObject>();
    PubSub<msg::ActivateTool> pub{this};

    Script() :
        engine{inject<script::Engine>{"toolengine"}},
        app{inject<script::ScriptObject>{"toolapp"}.shared<AppScriptObject>()}
        {
            tso->postInject();
    }

    void on(msg::ActivateTool& event) {
        if (Tool::active.lock().get() == this) {
            engine->raiseEvent({"toolactivate"});
        } else if (Tool::previous.lock().get() == this) {
            engine->raiseEvent({"tooldeactivate"});
        }
    }

    void begin(Surface* surface, const Path& points, U32 which) override {
        tso->surface->setWrapped(surface->shared_from_this());
        tso->points = &points;
        tso->which = which;
        app->setTarget(std::static_pointer_cast<script::ScriptObject>(tso));
        engine->raiseEvent({"toolstart"});
    }

    void update(Surface* surface, const Path& points) override {
        tso->surface->setWrapped(surface->shared_from_this());
        tso->points = &points;
        app->setTarget(std::static_pointer_cast<script::ScriptObject>(tso));
        engine->raiseEvent({"toolupdate"});
    }

    void end(Surface* surface, const Path& points) override {
        tso->surface->setWrapped(surface->shared_from_this());
        tso->points = &points;
        app->setTarget(std::static_pointer_cast<script::ScriptObject>(tso));
        engine->raiseEvent({"toolend"});
    }
};

static Tool::Shared<Script> reg{"script", {"noauto"}};
