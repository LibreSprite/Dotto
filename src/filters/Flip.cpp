// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Surface.hpp>
#include <filters/Filter.hpp>

class Surface;

class Flip : public Filter {
public:
    std::shared_ptr<PropertySet> getMetaProperties() override {return nullptr;}

    void run(std::shared_ptr<Surface> surface) override {
        auto data = surface->data();
        S32 width = surface->width();
        S32 height = surface->height();
        S32 half = height / 2;
        auto end = data + (height - 1) * width;
        for (S32 y = 0; y < half; ++y, data += width, end -= width) {
            for (S32 x = 0; x < width; ++x)
                std::swap(data[x], end[x]);
        }

        surface->setDirty(surface->rect());
    }
};

static Filter::Shared<Flip> reg{"flip vertical"};
