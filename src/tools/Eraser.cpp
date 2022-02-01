// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <tools/Pencil.hpp>

class Surface;

class Eraser : public Pencil {
public:
    void begin(Surface* surface, const Vector<Point2D>& points) override {
        selection = inject<Selection>{"new"};
        paint = inject<Command>{"paint"};
        selection->add(points.back().x, points.back().y, 255);
        paint->load({
                {"selection", selection},
                {"mode", "erase"},
                {"preview", true}
            });
        paint->run();
    }
};

static Tool::Shared<Eraser> erase{"eraser"};
