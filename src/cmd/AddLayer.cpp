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

class AddLayer : public Command {
    inject<ui::Node> editor{"activeeditor"};
    Property<String> cellType{this, "type", "bitmap"};
    S32 layerCount, frame;
    U32 prevLayer;
    std::shared_ptr<Cell> cell;

public:
    void undo() override {
        auto timeline = doc()->currentTimeline();
        if (!timeline)
            return;
        timeline->setCell(frame, layerCount, nullptr);
        editor->set("layer", prevLayer);
    }

    void run() override {
        if (!doc())
            return;
        auto timeline = doc()->currentTimeline();
        if (!timeline)
            return;
        if (!cell) {
            layerCount = timeline->layerCount();
            auto& ps = editor->getPropertySet();
            frame = ps.get<S32>("frame");
            while (layerCount > 1 && !timeline->getCell(frame, layerCount - 1))
                layerCount--;
            prevLayer = ps.get<S32>("layer");
            cell = inject<Cell>{cellType};
            if (!cell) {
                logE("AddLayer error: invalid cell type ", *cellType);
                return;
            }
            cell->getComposite()->resize(doc()->width(), doc()->height()); // TODO: support cells with different sizes
        }
        timeline->setCell(frame, layerCount, cell);
        editor->set("layer", layerCount);
        commit();
    }
};

static Command::Shared<AddLayer> cmd{"addlayer"};
