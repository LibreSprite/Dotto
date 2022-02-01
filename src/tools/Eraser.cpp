// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/line.hpp>
#include <common/Surface.hpp>
#include <doc/Selection.hpp>
#include <tools/Tool.hpp>

class Surface;

class Eraser : public Tool {
public:
    std::shared_ptr<Selection> selection;

    std::shared_ptr<Command> paint;

    virtual void begin(Surface* surface, const Vector<Point2D>& points) {
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

    virtual void update(Surface* surface, const Vector<Point2D>& points) {
        auto& end = points[points.size() - 1];
        auto& begin = points[points.size() - 2];
        line(begin, end, [=](const Point2D& point){
            selection->add(point.x, point.y, 255);
        });
        paint->run();
    }

    virtual void end(Surface* surface, const Vector<Point2D>& points) {
        paint->load({{"preview", false}});
        paint->run();
    }
};

static Tool::Shared<Eraser> erase{"eraser"};
