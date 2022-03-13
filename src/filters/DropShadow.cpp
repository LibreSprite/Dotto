// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Surface.hpp>
#include <doc/Selection.hpp>
#include <filters/Filter.hpp>
#include <tools/Tool.hpp>

class Surface;

class DropShadow : public Filter {
public:
    Property<S32> offsetX{this, "offset-x", 0};
    Property<S32> offsetY{this, "offset-y", 0};
    Property<Color> shadowColor{this, "shadow-color", "rgba{0,0,0,255}"};

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
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "color"},
                    {"label", shadowColor.name},
                    {"value", shadowColor.value = Tool::color}
                }));
        return meta;
    }

    void run(std::shared_ptr<Surface> surface) override {
        S32 offsetX = this->offsetX;
        S32 offsetY = this->offsetY;
        if (offsetX == 0 && offsetY == 0)
            return;

        S32 signX = offsetX < 0 ? -1 : offsetX > 0;
        S32 signY = offsetY < 0 ? -1 : offsetY > 0;
        S32 absX = std::abs(offsetX);
        S32 absY = std::abs(offsetY);

        auto write = surface->data();
        auto width = surface->width();
        auto height = surface->height();
        U8 colorR = shadowColor->r;
        U8 colorG = shadowColor->g;
        U8 colorB = shadowColor->b;
        U8 colorA = shadowColor->a;

        U32 startY = 0;
        U32 endY = height;
        if (offsetY < 0) {
            startY = -signY;
            endY = height;
        } else if (offsetY > 0) {
            startY = 0;
            endY = height - signY;
        }

        U32 startX = 0;
        U32 endX = width;
        if (offsetX < 0) {
            startX = -signX;
            endX = width;
        } else if (offsetX > 0) {
            startX = 0;
            endX = width - signX;
        }

        Vector<Surface::PixelType> src;
        while (absX || absY) {
            S32 dirX = 0, dirY = 0;
            if (absX) {
                absX--;
                dirX = signX;
            } else {
                absY--;
                dirY = signY;
            }
            src = surface->getPixels();
            for (U32 y = startY; y < endY; ++y) {
                for (U32 x = startX; x < endX; ++x) {
                    Color pixel{src[(y + dirY) * width + (x + dirX)]};
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
        }

        surface->setDirty();
    }
};

static Filter::Shared<DropShadow> reg{"dropshadow"};
