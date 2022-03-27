// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Document.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class PaletteScriptObject : public script::ScriptObject {
    std::weak_ptr<Palette> weak;
    PubSub<> pub{this};

public:
    void postInject() override {
        addProperty("length", [=]{
            if (auto pal = weak.lock()) {
                return S32(pal->size());
            }
            return 0;
        });

        addFunction("get", [=](int index){
            if (auto pal = weak.lock()) {
                auto color = pal->at(index);
                if (color)
                    return color->toU32();
            }
            return 0U;
        });
    }

    Value getWrapped() override {
        return weak.lock();
    }

    void setWrapped(const Value& vdoc) override {
        weak = std::shared_ptr<Palette>(vdoc);
    }
};

static script::ScriptObject::Shared<PaletteScriptObject> reg{typeid(std::shared_ptr<Palette>).name()};
