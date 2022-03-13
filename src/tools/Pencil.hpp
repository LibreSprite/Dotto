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

using namespace fs;

class Surface;

class Pencil : public Tool {
public:
    PubSub<msg::Flush> pub{this};
    std::shared_ptr<Selection> selection;
    std::shared_ptr<Command> paint;

    Property<String> shapeName{this, "shape", "%appdata/brushes/square.png", &Pencil::changeShape};
    std::shared_ptr<Surface> shape;
    F32 scale;
    Property<S32> size{this, "size", 1, &Pencil::invalidateMetaMenu};
    Property<F32> interval{this, "interval", 1.0f, &Pencil::invalidateMetaMenu};
    Property<F32> smoothing{this, "smoothing", 0.0f, &Pencil::invalidateMetaMenu};
    Property<bool> pressuresize{this, "pen-size", true, &Pencil::invalidateMetaMenu};
    Property<bool> pressurealpha{this, "pen-alpha", false, &Pencil::invalidateMetaMenu};
    Property<bool> pixelperfect{this, "pixelperfect", true, &Pencil::invalidateMetaMenu};
    Property<bool> HQ{this, "high-quality", true};

    bool wasInit = false;
    S32 prevPlotX = 0, prevPlotY = 0, prevPlotZ = 0;
    U32 which;
    Preview preview {.hideCursor = true};

    Preview* getPreview() override {
        return &preview;
    }

    void invalidateMetaMenu() override {
        Tool::invalidateMetaMenu();
    }

    void changeShape() {
        wasInit = true;
        shape = FileSystem::parse(shapeName);
        invalidateMetaMenu();
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
            if (rowCount >= 4) {
                meta->push(std::make_shared<PropertySet>(PropertySet{
                            {"widget", "brushrow"}
                        }));
                meta->push(std::make_shared<PropertySet>(PropertySet{
                            {"widget", "node"},
                            {"parent", "brushrow"},
                            {"width", "100%"},
                            {"min-width", "0px"}
                        }));
                rowCount = 0;
            }
            rowCount++;
            meta->push(std::make_shared<PropertySet>(PropertySet{
                        {"widget", "brushbutton"},
                        {"icon", brush},
                        {"parent", "brushrow"},
                        {"click", FunctionRef<void()>([=]{set("shape", brush);})}
                    }));
            meta->push(std::make_shared<PropertySet>(PropertySet{
                        {"widget", "node"},
                        {"parent", "brushrow"},
                        {"width", "100%"},
                        {"min-width", "0px"}
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
                    {"label", size.name},
                    {"value", size.value},
                    {"min", 1},
                    {"max", 128},
                    {"resolution", 1}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "range"},
                    {"label", smoothing.name},
                    {"value", smoothing.value},
                    {"min", -1.0f},
                    {"max", 1.0f},
                    {"resolution", 0.1}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "checkbox"},
                    {"label", HQ.name},
                    {"value", HQ.value}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "checkbox"},
                    {"label", pixelperfect.name},
                    {"value", pixelperfect.value}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "checkbox"},
                    {"label", pressuresize.name},
                    {"value", pressuresize.value}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "checkbox"},
                    {"label", pressurealpha.name},
                    {"value", pressurealpha.value}
                }));

        return meta;
    }

    void plot(const Point3D& point, bool force) {
        auto x = point.x;
        auto y = point.y;
        auto z = point.z / 255.0f;

        if (!force) {
            S32 dx = x - prevPlotX;
            S32 dy = y - prevPlotY;
            if (dx*dx+dy*dy < interval*interval)
                return;
        }

        prevPlotX = x;
        prevPlotY = y;
        prevPlotZ = z;

        if (smoothing < 0) {
            x += (rand() / F32(RAND_MAX) * 2.0f - 1.0f) * size * -smoothing;
            y += (rand() / F32(RAND_MAX) * 2.0f - 1.0f) * size * -smoothing;
        }

        Color color;
        U32 sw = shape->width();
        U32 sh = shape->height();
        S32 hsw = sw / 2;
        S32 hsh = sh / 2;

        if (pressuresize) {
            scale = F32(size) * z / shape->width();
        } else {
            scale = F32(size) / shape->width();
        }

        F32 alpha = pressurealpha ? z : 1.0f;

        if (size <= 1) {
            selection->add(x, y, 255 * alpha);
        } else if (scale < 1) {
            if (scale <= 0)
                return;
            if (!*HQ) {
                S32 step = 1 / scale;
                for (S32 sy = step/2; sy < sh; sy += step) {
                    for (S32 sx = step/2; sx < sw; sx += step) {
                        color = shape->getPixelUnsafe(sx, sy);
                        selection->add(x + (sx - hsw) * scale, y + (sy - hsh) * scale, color.a * alpha);
                    }
                }
            } else {
                for (S32 sy = 0; sy < sh; ++sy) {
                    for (S32 sx = 0; sx < sw; ++sx) {
                        color = shape->getPixelUnsafe(sx, sy);
                        selection->add(x + (sx - hsw) * scale, y + (sy - hsh) * scale, color.a * alpha * scale);
                    }
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
                            selection->add(ex, ey, color.a * alpha);
                        }
                    }
                }
            }
        }
    }

    Path applySmoothing(Surface* surface, const Path& points) {
        if (!points.size())
            return {};

        struct Point3DF {
            F32 x, y, z;

            Point3DF(const Point3D& p) : x(p.x), y(p.y), z(p.z) {}

            Point3DF(F32 x, F32 y, F32 z) : x{x}, y{y}, z{z} {}

            Point3D round() {
                return {S32(x + 0.5f), S32(y + 0.5f), S32(z + 0.5f)};
            }
        };

        Vector<Point3DF> smooth;
        smooth.reserve(points.size() * 2 - 1);
        smooth.push_back(points[0]);
        auto ref = points[0];
        for (U32 i = 1, size = points.size(); i < size; ++i) {
            Point3DF prev = ref;
            Point3D current = points[i];
            if (std::abs(prev.x - current.x) + std::abs(prev.y - current.y) < 2)
                continue;
            ref = current;
            Point3DF midpoint{
                (prev.x + current.x) / 2,
                (prev.y + current.y) / 2,
                (prev.z + current.z) / 2
            };
            smooth.push_back(midpoint);
            smooth.push_back(current);
        }

        auto current = smooth[0];
        auto next = smooth[1];
        for (U32 i = 1, size = smooth.size(); i < size - 1; ++i) {
            auto prev = current;
            current = next;
            next = smooth[i + 1];

            Point3DF tween{
                (prev.x + next.x) / 2,
                (prev.y + next.y) / 2,
                (prev.z + next.z) / 2
            };

            smooth[i].x = current.x * (1.0f - smoothing) + tween.x * smoothing;
            smooth[i].y = current.y * (1.0f - smoothing) + tween.y * smoothing;
            smooth[i].z = current.z * (1.0f - smoothing) + tween.z * smoothing;
        }

        Path rounded;
        for (auto& point : smooth)
            rounded.push_back(point.round());

        return rounded;
    }

    virtual void initPaint() {
        paint = inject<Command>{"paint"};
        paint->load({
                {"selection", selection},
                {"preview", true}
            });
        if (which == 0) {
            paint->set("cursor", true);
        } else if (which == 2) {
            paint->set("mode", "erase");
        }
    }

    void begin(Surface* surface, const Path& points, U32 which) override {
        this->which = which;
        if (!wasInit)
            changeShape();
        if (!shape)
            return;
        if (size < 1)
            *size = 1;
        if (interval <= 0)
            set("interval", shape->width() / 8.0f * scale);
        selection = inject<Selection>{"new"};
        initPaint();
        plot(points.back(), true);
        paint->run();
    }

    void update(Surface* surface, const Path& points) override {
        if (!shape)
            return;
        auto& end = points[points.size() - 1];
        auto& begin = points[points.size() - 2];
        line(begin, end, [&](S32 x, S32 y, S32 step, S32 max){
            F32 lerp = F32(step) / max;
            plot({x, y, S32(0.5f + begin.z * (1 - lerp) + end.z * lerp) }, false);
        });
        paint->run();
    }

    Path applyPixelPerfect(const Path src) {
        Path ret;
        ret.push_back(src.front());

        for (U32 it = 1, size = src.size(); it < size - 1; ++it) {
            auto x = src[it].x;
            auto y = src[it].y;
            auto px = src[it-1].x;
            auto py = src[it-1].y;
            auto nx = src[it+1].x;
            auto ny = src[it+1].y;
            it += (px == x || py == y)
                && (nx == x || ny == y)
                && px != nx
                && py != ny;
            ret.push_back(src[it]);
        }

        ret.push_back(src.back());

        return ret;
    }

    void end(Surface* surface, const Path& points) override {
        if (!shape)
            return;

        paint->load({{"preview", false}});
        paint->run();

        if (which == 0 || !surface || points.size() <= 2)
            return;

        if (smoothing <= 0 && !pixelperfect)
            return;

        Path copy = points;
        paint->undo();

        if (smoothing > 0)
            copy = applySmoothing(surface, copy);

        if (pixelperfect)
            copy = applyPixelPerfect(copy);

        Path segment;
        segment.push_back(copy[0]);
        begin(surface, segment, which);
        for (U32 i = 1, size = copy.size(); i < size; ++i) {
            segment.push_back(copy[i]);
            update(surface, segment);
        }
        end(nullptr, segment);
    }
};
