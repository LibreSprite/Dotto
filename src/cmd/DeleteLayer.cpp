// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/GroupCell.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <memory>

class DeleteLayer : public Command {
    inject<ui::Node> editor{"activeeditor"};

    struct Entry {
        std::shared_ptr<Cell> cell;
        std::shared_ptr<GroupCell> group;
        bool isActive;
        S32 layer;
    };
    Vector<Entry> cells;
    S32 next = 0;

public:
    void undo() override {
        for (auto& entry : cells) {
            entry.group->setCell(entry.layer, entry.cell);
            if (entry.isActive)
                editor->set("layer", entry.layer);
        }
    }

    bool willDelete(std::shared_ptr<Cell> cell) {
        for (auto& entity : cells) {
            if (entity.cell == cell)
                return true;
        }
        return false;
    }

    std::shared_ptr<Cell> findNextActive(std::shared_ptr<GroupCell> group) {
        std::shared_ptr<Cell> sibling, prevSibling, nextSibling;
        bool foundDel = false;
        for (U32 i = 0, max = group->layerCount(); i < max; ++i) {
            auto cell = group->getCell(i);
            if (!cell)
                continue;
            if (willDelete(cell)) {
                if (!prevSibling) {
                    prevSibling = sibling;
                }
                foundDel = true;
            } else if (auto childGroup = std::dynamic_pointer_cast<GroupCell>(cell)) {
                sibling = findNextActive(childGroup);
                if (sibling && foundDel)
                    return sibling;
            } else if (foundDel) {
                return cell;
            } else {
                sibling = cell;
            }
        }
        return nextSibling ?: prevSibling;
    }

    void run() override {
        if (!doc())
            return;

        if (cells.empty()) {
            auto selected = PubSub<>::pub(msg::PollSelectedCells{doc()}).cells;
            auto active = cell();
            bool willDeleteActive = false;

            for (auto cell : selected) {
                auto parent = cell.get();
                while (parent) {
                    if (parent == active.get()) {
                        willDeleteActive = true;
                        goto endSearch;
                    }
                    parent = parent->parent();
                }
            }
            endSearch:;

            for (auto cell : selected) {
                auto group = std::dynamic_pointer_cast<GroupCell>(cell->parent()->shared_from_this());
                if (!group)
                    continue;
                auto layer = group->getCellIndex(cell.get());
                if (layer == -1) {
                    logE("Cell isn't parent's child");
                    continue;
                }
                cells.push_back({
                        cell,
                        group,
                        cell == active,
                        layer
                    });
            }

            if (willDeleteActive) {
                auto timeline = doc()->currentTimeline();
                auto nextActive = findNextActive(std::dynamic_pointer_cast<GroupCell>(timeline->getCell(timeline->frame())));
                if (!nextActive) {
                    logE("Can't delete last layer");
                    return;
                }
                next = dynamic_cast<GroupCell*>(nextActive->parent())->getCellIndex(nextActive.get());
            }
        }
        for (auto& entry : cells) {
            entry.group->setCell(entry.layer, nullptr);
        }
        editor->set("layer", next);
        commit();
    }
};

static Command::Shared<DeleteLayer> cmd{"deletelayer"};
