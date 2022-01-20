// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Surface.hpp>
#include <doc/Selection.hpp>
#include <filters/Filter.hpp>

class Surface;

class DropShadown : public Filter {
public:
    Property<S32> offsetX{this, "offset-x", 0};
    Property<S32> offsetY{this, "offset-y", 0};
    Property<Color> shadowColor{this, "shadow-color", "rgba{0,0,0,255}"};
    Property<U32> blur{this, "blur", 0};

    std::shared_ptr<PropertySet> getMetaProperties() override {
        auto meta = Filter::getMetaProperties();
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "number"},
                    {"label", offsetX.name},
                    {"value", offsetX.value}
                }));
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "number"},
                    {"label", offsetY.name},
                    {"value", offsetY.value}
                }));
        return meta;
    }

    void run(std::shared_ptr<Surface> surface) override {
        S32 offsetX = this->offsetX;
        S32 offsetY = this->offsetY;
        if (offsetX == 0 && offsetY == 0)
            return;

        auto write = surface->data();
        Vector<Surface::PixelType> src{write, surface->data() + surface->dataSize()};
        auto width = surface->width();
        auto height = surface->height();
        U8 colorR = shadowColor->r;
        U8 colorG = shadowColor->g;
        U8 colorB = shadowColor->b;
        U8 colorA = shadowColor->a;
        if (!blur) {
            U32 startY = 0;
            U32 endY = height;
            if (offsetY < 0) {
                startY = -offsetY;
                endY = height;
            } else if (offsetY > 0) {
                startY = 0;
                endY = height - offsetY;
            }

            U32 startX = 0;
            U32 endX = width;
            if (offsetX < 0) {
                startX = -offsetX;
                endX = width;
            } else if (offsetX > 0) {
                startX = 0;
                endX = width - offsetX;
            }

            for (U32 y = startY; y < endY; ++y) {
                for (U32 x = startX; x < endX; ++x) {
                    Color pixel{src[(y + offsetY) * width + (x + offsetX)]};
                    Color previous{src[y * width + x]};
                    F32 alpha = previous.a / 255.0f;
                    U8 shadownAlpha = colorA * (pixel.a / 255.0f);
                    write[y * width + x] = Color{
                        U8(colorR * (1 - alpha) + previous.r * alpha),
                        U8(colorG * (1 - alpha) + previous.g * alpha),
                        U8(colorB * (1 - alpha) + previous.b * alpha),
                        U8(shadownAlpha + alpha * (255 - shadownAlpha)),
                    }.toU32();
                }
            }
        } else {
        }
        surface->setDirty();
    }
};

static Filter::Shared<DropShadown> reg{"dropshadow"};
