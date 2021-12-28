// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <memory>
#include <sstream>

#include <common/Surface.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class SurfaceScriptObject : public script::ScriptObject {
    std::shared_ptr<Surface> surface;

public:
    SurfaceScriptObject() {
        addProperty("width",
                    [=]{return surface->width();},
                    [=](U32 width){
                        surface->resize(width, surface->height());
                        return width;
                    });

        addProperty("height",
                    [=]{return surface->height();},
                    [=](U32 height){
                        surface->resize(surface->width(), height);
                        return height;
                    });

        addMethod("setPixel", this, &SurfaceScriptObject::setPixel);
        addMethod("getPixel", this, &SurfaceScriptObject::getPixel);
    }

    Value getWrapped() override {
        return surface;
    }

    void setWrapped(const Value& value) override {
        surface = value;
    }

    void setPixel(S32 x, S32 y, U32 color) {
        surface->setPixel(x, y, color);
    }

    U32 getPixel(S32 x, S32 y) {
        return surface->getPixel(x, y).toU32();
    }
};

static script::ScriptObject::Shared<SurfaceScriptObject> regType{typeid(std::shared_ptr<Surface>).name()};
