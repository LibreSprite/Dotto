// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Surface.hpp>
#include <filters/Filter.hpp>

class Surface;

class Mirror : public Filter {
public:
    String category() override {return "transform";}

    void run(std::shared_ptr<Surface> surface) override {
        auto data = surface->data();
        S32 width = surface->width();
        S32 half = width / 2;
        S32 height = surface->height();

        for (S32 y = 0; y < height; ++y, data += width) {
            for (S32 x = 0; x < half; ++x)
                std::swap(data[x], data[width - x - 1]);
        }

        surface->setDirty(surface->rect());
    }
};

static Filter::Shared<Mirror> reg{"flip horizontal"};
