// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/line.hpp>
#include <common/Surface.hpp>
#include <doc/Document.hpp>
#include <doc/Selection.hpp>
#include <tools/Tool.hpp>

class Bucket : public  Tool {
public:
    Property<S32> threshold{this, "threshold", 0};
    Property<bool> proportional{this, "proportional", false};
    Property<bool> contiguous{this, "contiguous", true};

    virtual std::shared_ptr<PropertySet> getMetaProperties() {
        auto meta = Tool::getMetaProperties();
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "number"},
                    {"label", threshold.name},
                    {"value", threshold.value}
                }));
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "checkbox"},
                    {"label", proportional.name},
                    {"value", proportional.value}
                }));
        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "checkbox"},
                    {"label", contiguous.name},
                    {"value", contiguous.value}
                }));
        return meta;
    }

    virtual void begin(Surface* surface, const Vector<Point2D>& points, U32 which) {
        auto targetColor = surface->getPixel(points.back().x, points.back().y);
        if (targetColor == color)
            return;

        S32 threshold = std::max(0, std::min<S32>(this->threshold, 255));
        threshold *= threshold;
        bool proportional = this->proportional && threshold;

        inject<Selection> selection{"new"};
        selection->clear();
        S32 width = surface->width();
        S32 height = surface->height();

        if (*contiguous ^ (which == 1)) {
            Vector<Point2D> queue = points;
            while (!queue.empty()) {
                S32 x = queue.back().x;
                S32 y = queue.back().y;
                queue.pop_back();
                if (x < 0 || y < 0 || x >= width || y >= height || selection->get(x, y))
                    continue;

                auto srcPixel = surface->getPixelUnsafe(x, y);
                S32 distance = threshold - targetColor.distanceSquared(srcPixel);
                if (distance < 0 || (distance == 0 && threshold))
                    continue;

                U8 amount = proportional ? distance * 255 / threshold : 255;
                if (!amount)
                    amount = 1;
                selection->add(x, y, amount);

                if (x > 0) queue.push_back({x - 1, y});
                if (y > 0) queue.push_back({x, y - 1});
                if (y < height - 1) queue.push_back({x, y + 1});
                if (x < width - 1) queue.push_back({x + 1, y});
            }
        } else {
            for (U32 y = 0; y < height; ++y) {
                for (U32 x = 0; x < width; ++x) {
                    auto srcPixel = surface->getPixelUnsafe(x, y);
                    S32 distance = threshold - targetColor.distanceSquared(srcPixel);
                    if (distance < 0 || (distance == 0 && threshold))
                        continue;

                    U8 amount = proportional ? distance * 255 / threshold : 255;
                    if (!amount)
                        amount = 1;
                    selection->add(x, y, amount);
                }
            }
        }

        inject<Command> paint{"paint"};
        paint->set("selection", selection.shared());
        paint->run();
    }
};

static Tool::Shared<Bucket> bucket{"bucket"};
