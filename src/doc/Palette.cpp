// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Palette.hpp>
#include <common/Octree.hpp>

class PaletteImpl : public Palette {
public:
    void loadFromSurface(Surface& surface, U32 maxColors) override {
        ColorOctree tree{{0, 0, 0, 0}, {255, 255, 255, 255}};
        for (auto& pixel : surface.getPixels()) {
            tree.add(pixel);
        }

        colors.clear();
        colors.resize(maxColors);
        tree.collect(colors);

        if (colors.size() > 1) {
            std::sort(colors.begin(), colors.end(), [](Color& a, Color& b){return a.toU32() < b.toU32();});
            auto prev = *colors.begin();
            for (auto it = colors.begin() + 1; it != colors.end();) {
                if (*it == prev) {
                    it = colors.erase(it);
                } else {
                    prev = *it++;
                }
            }
        }

        // int i = 0;
        // for (auto& color : colors) {
        //     logI("Color #", i, ": ", color);
        //     i++;
        // }
    }

    U32 findClosestColorIndex(const Color& color) override {
        if (colors.empty())
            return color.r;
        U32 closestIndex = 0;
        auto closestDistance = colors[0].distanceSquared(color);
        if (!closestDistance)
            return closestIndex;
        for (U32 index = 1, size = colors.size(); index < size; ++index) {
            auto distance = colors[index].distanceSquared(color);
            if (distance < closestDistance) {
                if (!distance)
                    return index;
                closestIndex = index;
                closestDistance = distance;
            }
        }
        return closestIndex;
    }
};

static Palette::Shared<PaletteImpl> reg{"new"};
