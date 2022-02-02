// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <cmd/Command.hpp>
#include <common/FunctionRef.hpp>
#include <common/Messages.hpp>
#include <common/PropertySet.hpp>
#include <common/PubSub.hpp>
#include <common/line.hpp>
#include <common/Surface.hpp>
#include <doc/Selection.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <tools/Tool.hpp>

class Surface;

class Pencil : public Tool {
public:
    PubSub<msg::Flush> pub{this};
    std::shared_ptr<Selection> selection;
    std::shared_ptr<Command> paint;

    Property<String> shapeName{this, "shape", "%appdata/brushes/square.png", &Pencil::changeShape};
    std::shared_ptr<Surface> shape;
    Property<F32> scale{this, "scale", 1.0f};
    Property<F32> interval{this, "interval", 1.0f};
    bool wasInit = false;
    S32 prevPlotX = 0, prevPlotY = 0;

    void changeShape() {
        wasInit = true;
        shape = FileSystem::parse(shapeName);
        if (interval <= 0)
            *interval = shape->width() / 8.0f * scale;
    }

    void on(msg::Flush& flush) {
        flush.hold(shape);
    }

    Vector<String> getBrushes() {
        inject<FileSystem> fs;
        Vector<String> paths;
        auto brushes = fs->find("%appdata/brushes", "dir")->get<Folder>();
        if (brushes) {
            brushes->forEach([&](std::shared_ptr<FSEntity> child) {
                if (child->isFile()) {
                    paths.push_back("%appdata/brushes/" + child->get<File>()->name());
                }
            });
        }
        return paths;
    }

    std::shared_ptr<PropertySet> getMetaProperties() override {
        if (!wasInit)
            changeShape();
        auto meta = Tool::getMetaProperties();

        U32 rowCount = ~U32{};
        for (auto& brush : getBrushes()) {
            if (rowCount >= 5) {
                meta->push(std::make_shared<PropertySet>(PropertySet{
                            {"widget", "row"},
                            {"id", "brushrow"},
                            {"height", "64px"}
                        }));
                meta->push(std::make_shared<PropertySet>(PropertySet{
                            {"widget", "node"},
                            {"parent", "brushrow"},
                            {"width", "100%"}
                        }));
                rowCount = 0;
            }
            rowCount++;
            meta->push(std::make_shared<PropertySet>(PropertySet{
                        {"widget", "button"},
                        {"icon", brush},
                        {"icon-multiply", "rgb{64,128,255}"},
                        {"width", "64px"},
                        {"parent", "brushrow"},
                        {"click", FunctionRef<void()>([=]{set("shape", brush);})}
                    }));
            meta->push(std::make_shared<PropertySet>(PropertySet{
                        {"widget", "node"},
                        {"parent", "brushrow"},
                        {"width", "100%"}
                    }));
        }

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "range"},
                    {"label", interval.name},
                    {"value", interval.value},
                    {"min", 1.0f},
                    {"max", 100.0f},
                    {"resolution", 1.0f}
                }));
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "range"},
                    {"label", scale.name},
                    {"value", scale.value},
                    {"min", 0.01f},
                    {"max", 4.0f},
                    {"resolution", 0.01f}
                }));
        return meta;
    }

    void plot(S32 x, S32 y, bool force) {
        if (!force) {
            S32 dx = x - prevPlotX;
            S32 dy = y - prevPlotY;
            if (dx*dx+dy*dy < interval*interval)
                return;
        }
        prevPlotX = x;
        prevPlotY = y;
        Color color;
        U32 sw = shape->width();
        U32 sh = shape->height();
        S32 hsw = sw / 2;
        S32 hsh = sh / 2;
        if (scale < 1) {
            S32 step = 1 / scale;
            for (S32 sy = step/2; sy < sh; sy += step) {
                for (S32 sx = step/2; sx < sw; sx += step) {
                    color = shape->getPixelUnsafe(sx, sy);
                    selection->add(x + (sx - hsw) * scale, y + (sy - hsh) * scale, color.a);
                }
            }
        } else {
            for (S32 sy = 0; sy < sh; ++sy) {
                S32 oy = y + (sy - hsh) * scale;
                S32 maxoy = oy + scale;
                for (S32 sx = 0; sx < sw; ++sx) {
                    color = shape->getPixelUnsafe(sx, sy);
                    S32 ox = x + (sx - hsw) * scale;
                    S32 maxox = ox + scale;
                    for (S32 ey = oy; ey < maxoy; ++ey) {
                        for (S32 ex = ox; ex < maxox; ++ex) {
                            selection->add(ex, ey, color.a);
                        }
                    }
                }
            }
        }
    }

    virtual void initPaint() {
        paint = inject<Command>{"paint"};
        paint->load({
                {"selection", selection},
                {"preview", true}
            });
    }

    void begin(Surface* surface, const Vector<Point2D>& points) override {
        if (!wasInit)
            changeShape();
        if (!shape)
            return;
        selection = inject<Selection>{"new"};
        initPaint();
        plot(points.back().x, points.back().y, true);
        paint->run();
    }

    void update(Surface* surface, const Vector<Point2D>& points) override {
        if (!shape)
            return;
        auto& end = points[points.size() - 1];
        auto& begin = points[points.size() - 2];
        line(begin, end, [=](const Point2D& point){
            plot(point.x, point.y, false);
        });
        paint->run();
    }

    void end(Surface* surface, const Vector<Point2D>& points) override {
        if (!shape)
            return;
        paint->load({{"preview", false}});
        paint->run();
    }
};
