// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <doc/Cell.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>

class DeleteLayer : public Command {
    inject<ui::Node> editor{"activeeditor"};
    Property<String> cellType{this, "type", "bitmap"};
    S32 layer, frame, next;
    std::shared_ptr<Cell> cell;

public:
    void undo() override {
        auto timeline = doc()->currentTimeline();
        if (!timeline)
            return;
        timeline->setCell(frame, layer, cell);
        editor->set("layer", layer);
    }

    void run() override {
        if (!doc())
            return;
        auto timeline = doc()->currentTimeline();
        if (!timeline)
            return;

        if (!cell) {
            auto& ps = editor->getPropertySet();
            frame = ps.get<S32>("frame");
            layer = ps.get<S32>("layer");

            S32 layerCount = timeline->layerCount();
            next = layer - 1;
            while (next > 0 && !timeline->getCell(frame, next))
                next--;

            if (next < 0 || !timeline->getCell(frame, next)) {
                next = layer + 1;
                for (;next < layerCount && !timeline->getCell(frame, next); ++next);
            }

            if (!timeline->getCell(frame, next))
                return;

            cell = timeline->getCell(frame, layer);
            if (!cell)
                return;
        }

        timeline->setCell(frame, layer, nullptr);
        editor->set("layer", next);
        commit();
    }
};

static Command::Shared<DeleteLayer> cmd{"deletelayer"};
