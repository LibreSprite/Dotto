// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Selection.hpp>
#include <script/api/ModelScriptObject.hpp>
#include <script/Engine.hpp>

class SelectionScriptObject : public script::ScriptObject {
    std::weak_ptr<Selection> weak;

public:
    SelectionScriptObject() {
        addFunction("clear", [=]{
            if (auto selection = weak.lock())
                selection->clear();
            return true;
        });
        addFunction("add", [=](U32 x, U32 y, U32 amount) {
            if (auto selection = weak.lock())
                selection->add(x, y, amount);
            return true;
        });

        addFunction("subtract", [=](U32 x, U32 y, U32 amount) {
            if (auto selection = weak.lock())
                selection->subtract(x, y, amount);
            return true;
        });
    }

    Value getWrapped() override {
        return weak.lock();
    }

    void setWrapped(const Value& vmodel) override {
        if (vmodel.has<std::shared_ptr<Selection>>())
            weak = vmodel.get<std::shared_ptr<Selection>>();
    }
};

static script::ScriptObject::Shared<SelectionScriptObject> reg{typeid(std::shared_ptr<Selection>).name()};
