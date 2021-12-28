// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/line.hpp>
#include <common/Surface.hpp>
#include <tools/Tool.hpp>

class Surface;

class Pencil : public  Tool {
public:
    virtual void begin(Surface* surface, const Vector<Point>& points) {
        auto& point = points.back();
        surface->setPixel(point.x, point.y, color);
    }

    virtual void update(Surface* surface, const Vector<Point>& points) {
        auto& end = points[points.size() - 1];
        auto& begin = points[points.size() - 2];
        line(begin, end, [=](Point point){
            surface->setPixel(point.x, point.y, color);
        });
    }

    virtual void end(Surface* surface, const Vector<Point>& points) {}
};

static Tool::Shared<Pencil> pencil{"brush"};
