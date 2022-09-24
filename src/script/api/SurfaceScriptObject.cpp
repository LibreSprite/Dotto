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
        addMethod("setPixels", this, &SurfaceScriptObject::setPixels);
        addMethod("blit", this, &SurfaceScriptObject::blit);
    }

    Value getWrapped() override {
        return surface;
    }

    void setWrapped(const Value& value) override {
        surface = value;
    }

    void setPixels(script::Value::Buffer& data) {
        if (data.size() == surface->width() * surface->height() * 4) {
            std::copy(data.data(), data.data() + data.size(), surface->data());
            surface->setDirty({0, 0, surface->width(), surface->height()});
        } else if (data.size() == surface->width() * surface->height() * 3) {
            auto src = data.data();
            auto end = data.size();
            Color color;
            for (auto i = 0, j = 0; i < end;) {
                color.r = src[i++];
                color.g = src[i++];
                color.b = src[i++];
                surface->data()[j++] = color.toU32();
            }
            surface->setDirty({0, 0, surface->width(), surface->height()});
        }
    }

    void blit(script::ScriptObject* sotarget, S32 x, S32 y) {
        if (!sotarget)
            return;
        auto wtarget = sotarget->getWrapped();
        auto target = wtarget.get<std::shared_ptr<Surface>>();
        if (!target) {
            logI("never expect the ", wtarget.typeName());
            return;
        }
        if (x >= S32(target->width()) || y >= S32(target->height()))
            return;
        if (x <= -S32(surface->width()) || y <= -S32(surface->height()))
            return;
        auto maxX = std::min<S32>(x + surface->width(), target->width());
        auto maxY = std::min<S32>(y + surface->height(), target->height());
        auto minX = std::max<S32>(x, 0);
        auto minY = std::max<S32>(y, 0);
        auto src = surface->data() + (minY - y) * surface->width();
        auto dest = target->data() + (minY) * target->width();
        for (auto dy = minY; dy < maxY; ++dy, src += surface->width(), dest += target->width()) {
            for (auto dx = minX; dx < maxX; ++dx) {
                dest[dx] = src[dx - x];
            }
        }
        target->setDirty({minX, minY, U32(maxX - minX), U32(maxY - minY)});
    }

    void setPixel(S32 x, S32 y, U32 color) {
        surface->setPixel(x, y, color);
    }

    U32 getPixel(S32 x, S32 y) {
        return surface->getPixel(x, y).toU32();
    }
};

static script::ScriptObject::Shared<SurfaceScriptObject> regType{typeid(std::shared_ptr<Surface>).name()};
