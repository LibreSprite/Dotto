// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Surface.hpp>
#include <doc/Selection.hpp>
#include <filters/Filter.hpp>
#include <tools/Tool.hpp>

class Surface;

class Blur : public Filter {
public:
    Property<S32> radiusX{this, "radius-x", 10};
    Property<S32> radiusY{this, "radius-y", 10};

    std::shared_ptr<PropertySet> getMetaProperties() override {
        auto meta = Filter::getMetaProperties();

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "range"},
                    {"label", radiusX.name},
                    {"value", radiusX.value},
                    {"min", 0},
                    {"max", 100},
                    {"resolution", 1}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "range"},
                    {"label", radiusY.name},
                    {"value", radiusY.value},
                    {"min", 0},
                    {"max", 100},
                    {"resolution", 1}
                }));

        return meta;
    }

    void run(std::shared_ptr<Surface> surface) override {
        S32 radiusX = this->radiusX;
        S32 radiusY = this->radiusY;
        if (radiusX == 0 && radiusY == 0)
            return;

        auto data = surface->data();
        S32 width = surface->width();
        S32 height = surface->height();

        Vector<Color> line;

        if (radiusX) {
            F32 sum = 1.0f / (radiusX * 2 + 1);
            line.resize(width);
            for (S32 y = 0; y < height; ++y) {
                Color c;
                auto image = data + width * y;
                for (S32 x = 0; x < width; ++x)
                    line[x] = image[x];
                for (S32 x = 0; x < width; ++x) {
                    U32 r = 0;
                    U32 g = 0;
                    U32 b = 0;
                    U32 a = 0;
                    for (S32 w = x - radiusX; w <= x + radiusX; ++w) {
                        S32 rw = w;
                        while (rw < 0) rw = width + rw;
                        while (rw >= width) rw -= width;
                        r += line[rw].r;
                        g += line[rw].g;
                        b += line[rw].b;
                        a += line[rw].a;
                    }
                    c.r = r * sum;
                    c.g = g * sum;
                    c.b = b * sum;
                    c.a = a * sum;
                    image[x] = c.toU32();
                }
            }
        }

        if (radiusY) {
            F32 sum = 1.0f / (radiusY * 2 + 1);
            line.resize(height);
            for (S32 x = 0; x < width; ++x) {
                Color c;
                for (S32 y = 0; y < height; ++y)
                    line[y] = data[y * width + x];
                for (S32 y = 0; y < height; ++y) {
                    U32 r = 0;
                    U32 g = 0;
                    U32 b = 0;
                    U32 a = 0;
                    for (S32 w = y - radiusY; w <= y + radiusY; ++w) {
                        S32 rw = w;
                        while (rw < 0) rw = height + rw;
                        while (rw >= height) rw -= height;
                        c = line[rw];
                        r += c.r;
                        g += c.g;
                        b += c.b;
                        a += c.a;
                    }
                    c.r = r * sum;
                    c.g = g * sum;
                    c.b = b * sum;
                    c.a = a * sum;
                    data[y * width + x] = c.toU32();
                }
            }
        }

        surface->setDirty();
    }
};

static Filter::Shared<Blur> reg{"gaussian-blur"};
