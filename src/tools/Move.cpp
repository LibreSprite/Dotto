/*
// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "layer/Layer.hpp"
#include <cmd/Command.hpp>
#include <common/line.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <doc/BitmapCell.hpp>
#include <doc/Document.hpp>
#include <doc/Selection.hpp>
#include <doc/GroupCell.hpp>
#include <tools/Tool.hpp>

class Move : public  Tool {
public:
    PubSub<> pub{this};
    std::shared_ptr<Surface> lifted;

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
        S32 offsetX = points.back().x - points.front().x;
        S32 offsetY = points.back().y - points.front().y;
        points.front() = points.back();
        preview.overlay->move(offsetX, offsetY);
    }

    void end(Surface* surface, Path& points) override {
        // if (!which)
        //     return;
        // inject<Document> doc{"activedocument"};
        // auto timeline = doc->currentTimeline();
        // if (auto cell = dynamic_cast<BitmapCell*>(timeline->getCell().get())) {
        //     cell->setSelection(preview.overlay.get());
        // }
        // preview.overlay->clear();
    }

    void begin(Surface* surface, Path& points, U32 which) override {
        this->which = which;
        if (!which)
            return;
        inject<Cell> cell{"activecell"};
        if (auto bitmapCell = dynamic_cast<BitmapCell*>(cell.get())) {
            beginBitmapMove(bitmapCell);
        }
    }

    void applyBitmapMove(GroupCell* group) {

    }

    std::shared_ptr<GroupCell> findBitmapMoveGroup(Document* doc) {
        for (auto cell : doc->cells()) {
            auto parent = group->getParent();
            if (!parent || cell->getName() != ".MoveToolTmp")
                continue;
            if (auto group = dynamic_cast<GroupCell*>(cell)) {
                applyBitmapMove(group);
                return std::static_pointer_cast<GroupCell>(group->shared_from_this());
            }
        }

        auto group = inject<Cell>{"group"}.shared<GroupCell>();
        group->setName(".MoveToolTmp");
        return group;
    }

    void beginBitmapMove(BitmapCell* cell) {
        auto tmpGroup = findBitmapMoveGroup(cell->document());

        // auto selection = cell->getSelection();
        // if (!selection) {
        //     auto lock = doc->getHistoryLock();
        //     inject<Command>{"selectall"}->run();
        //     selection = cell->getSelection();
        // }
        // if (!selection)
        //     return;


        // auto bounds = selection->getTrimmedBounds();
        // lifted = std::make_shared<Surface>();
        // lifted->resize(bounds.right(), bounds.bottom());
        // selection->write(lifted.get(), selection->read(cell->getComposite()));

        // area = selection->getBounds();
        // *preview.overlay = *selection;
        // cell->setSelection(nullptr);
    }
};

static Tool::Shared<Move> move{"move"};
/* */
