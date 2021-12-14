// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/types.hpp>
#include <common/Color.hpp>

using Palette = Vector<Color>;

inline U32 findClosestColorIndex(const Palette& palette, const Color& color) {
    if (palette.empty())
        return color.r;
    U32 closestIndex = 0;
    auto closestDistance = palette[0].distanceSquared(color);
    if (!closestDistance)
        return closestIndex;
    for (U32 index = 1, size = palette.size(); index < size; ++index) {
        auto distance = palette[index].distanceSquared(color);
        if (distance < closestDistance) {
            if (!distance)
                return index;
            closestIndex = index;
            closestDistance = distance;
        }
    }
    return closestIndex;
}

inline Color findClosestColor(const Palette& palette, const Color& color) {
    if (palette.empty())
        return {color.r, color.r, color.r, 255};
    return palette[findClosestColorIndex(palette, color)];
}
