// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/line.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <doc/BitmapCell.hpp>
#include <doc/Document.hpp>
#include <doc/Selection.hpp>
#include <tools/Tool.hpp>

class Marquee : public  Tool {
public:
    PubSub<> pub{this};

    Preview preview {
        .hideCursor = false,
        .overlay = inject<Selection>{"new"},
        .overlayColor = Color{200, 200, 200, 200},
        .altColor = Color{55, 55, 55, 200},
        .draw = Tool::Preview::drawOutlineAnts
    };
    Preview* getPreview() override {return &preview;}

    Rect area;
    U32 which;

    void update(Surface* surface, Path& points) override {
        if (points.size() < 2 || !which) {
            return;
        }
        if (points.size() >= 2) {
            area.x = points.front().x;
            area.y = points.front().y;
            area.width = points.back().x - area.x;
            area.height = points.back().y - area.y;
            if (S32(area.width) < 0) {
                area.x += area.width;
                area.width = -area.width;
            }
            if (S32(area.height) < 0) {
                area.y += area.height;
                area.height = -area.height;
            }
        }

        if (which == 1)
            preview.overlay->clear();
        if (which == 2)
            preview.overlay->subtract(area, 255);
        else
            preview.overlay->add(area, 255);
    }

    void end(Surface* surface, Path& points) override {
        if (!which)
            return;
        inject<Document> doc{"activedocument"};
        auto timeline = doc->currentTimeline();
        if (auto cell = dynamic_cast<BitmapCell*>(timeline->getCell().get())) {
            pub(msg::PreModifySelection{cell->getSelection()});
            cell->setSelection(preview.overlay.get());
        }
        preview.overlay->clear();
    }

    void begin(Surface* surface, Path& points, U32 which) override {
        if (which) {
            inject<Document> doc{"activedocument"};
            auto timeline = doc->currentTimeline();
            auto cell = dynamic_cast<BitmapCell*>(timeline->getCell().get());
            if (!cell)
                return;
            if (auto selection = cell->getSelection()) {
                if (which >= 2) {
                    logI(selection->getBounds());
                    *preview.overlay = *selection;
                }
                pub(msg::PreModifySelection{selection});
                cell->setSelection(nullptr);
            }
        }
        this->which = which;
    }
};

static Tool::Shared<Marquee> marquee{"marquee"};
