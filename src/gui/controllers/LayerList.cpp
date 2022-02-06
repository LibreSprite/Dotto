// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <filters/Filter.hpp>

class LayerList : public ui::Controller {
    PubSub<msg::ActivateEditor, msg::ActivateLayer, msg::ActivateFrame> pub{this};
    Vector<std::shared_ptr<ui::Node>> nodePool;

public:

    void on(msg::ActivateEditor&) {update();}
    void on(msg::ActivateFrame&) {update();}
    void on(msg::ActivateLayer&) {update();}

    void update() {
        inject<ui::Node> editor{"activeeditor"};
        if (!editor) {
            logI("No editor");
            return;
        }
        auto& ps = editor->getPropertySet();
        auto frame = ps.get<S32>("frame");
        auto currentLayer = ps.get<S32>("layer");
        auto doc = ps.get<std::shared_ptr<Document>>("doc");
        if (!doc) {
            logI("No document");
            return;
        }
        auto timeline = doc->currentTimeline();
        auto layerCount = timeline->layerCount();

        while (layerCount > nodePool.size()) {
            auto item = ui::Node::fromXML("layerlistitem");
            node()->addChild(item);
            nodePool.push_back(item);
        }

        while (layerCount < nodePool.size()) {
            nodePool.back()->remove();
            nodePool.pop_back();
        }

        S32 height = 0;
        S32 width = node()->innerWidth();
        for (U32 i = 0; i < layerCount; ++i) {
            std::shared_ptr<ui::Node> item = nodePool[i];
            U32 layer = layerCount - 1 - i;
            auto cell = timeline->getCell(frame, layer);
            if (!cell) {
                item->remove();
                continue;
            }
            if (!item->getParent())
                node()->addChild(item);
            auto surface = cell ? cell->getComposite()->shared_from_this() : nullptr;
            auto aspect = surface ? surface->width() / F32(surface->height()) : 4.0f;
            S32 itemHeight = width / aspect;

            auto maxHeight = item->maxHeight->toPixel(10000, 10000);
            Rect previewPadding;
            if (itemHeight > maxHeight) {
                F32 previewWidth = maxHeight * aspect;
                previewPadding.x = previewWidth/2;
                previewPadding.width = previewWidth/2 + 0.5f;
                itemHeight = maxHeight;
            }

            item->load({
                    {"preview", surface},
                    {"height", itemHeight},
                    {"preview-padding", previewPadding},
                    {"click", "ActivateLayer layer=" + std::to_string(layer)},
                    {"state", layer == currentLayer ? "active" : "enabled"},
                    {"cell", cell},
                });
            height += itemHeight;
        }

        if (height)
            node()->set("height", height);
    }
};

static ui::Controller::Shared<LayerList> layerlist{"layerlist"};
