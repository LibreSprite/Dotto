// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/line.hpp>
#include <common/Surface.hpp>
#include <tools/Tool.hpp>

class Surface;

class Bucket : public  Tool {
public:
    virtual void begin(Surface* surface, const Vector<Point>& points) {
        auto targetColor = surface->getPixel(points.back().x, points.back().y);
        if (targetColor == color)
            return;
        S32 width = surface->width();
        S32 height = surface->height();
        Vector<Point> queue = points;
        while (!queue.empty()) {
            S32 x = queue.back().x;
            S32 y = queue.back().y;
            queue.pop_back();
            surface->setPixelUnsafe(x, y, color.toU32());

            if (x > 0 && surface->getPixel(x - 1, y) == targetColor) queue.push_back({x - 1, y});
            if (y > 0 && surface->getPixel(x, y - 1) == targetColor) queue.push_back({x, y - 1});
            if (y < height - 1 && surface->getPixel(x, y + 1) == targetColor) queue.push_back({x, y + 1});
            if (x < width - 1 && surface->getPixel(x + 1, y) == targetColor) queue.push_back({x + 1, y});
        }
    }

    virtual void update(Surface* surface, const Vector<Point>& points) {}

    virtual void end(Surface* surface, const Vector<Point>& points) {}
};

static Tool::Shared<Bucket> bucket{"bucket"};
