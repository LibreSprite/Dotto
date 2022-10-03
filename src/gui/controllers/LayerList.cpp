// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/FunctionRef.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/System.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <filters/Filter.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <limits>

class LayerList : public ui::Controller {
    PubSub<msg::ActivateDocument,
           msg::ActivateLayer,
           msg::ActivateFrame,
           msg::ModifyGroup,
           msg::PollSelectedCells> pub{this};
    Vector<std::shared_ptr<ui::Node>> nodePool;

public:

    void on(msg::ActivateDocument&) {update();}
    void on(msg::ActivateFrame&) {update();}
    void on(msg::ActivateLayer&) {update();}
    void on(msg::ModifyGroup&) {update();}

    void on(msg::PollSelectedCells& poll) {
        for (auto item : nodePool) {
            if (layerState(item) == "enabled")
                continue;
            auto prop = item->get("cell");
            if (!prop)
                continue;
            auto cell = prop->get<std::shared_ptr<Cell>>();
            if (!cell)
                continue;
            if (cell->document() != poll.doc.get())
                continue;
            poll.cells.push_back(cell);
        }
    }

    void update() {
        inject<ui::Node> editor{InjectSilent::Yes, "activeeditor"};
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

            auto surface = cell ? cell->getComposite()->shared_from_this() : nullptr;
            auto aspect = surface ? surface->width() / F32(surface->height() ?: 1) : 1.0f;
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
                    {"layer-number", layer},
                    {"click", FunctionRef<void()>([=]{
                        auto& keys = inject<System>{}->getPressedKeys();
                        auto parent = item->getParent();
                        if (keys.count("LSHIFT") || keys.count("RSHIFT")) {
                            if (auto [min, max] = getSelectionRange(); max != -1) {
                                for (S32 j = 0; j < nodePool.size(); ++j) {
                                    auto node = nodePool[j];
                                    auto index = layerNumber(node);
                                    if (index == -1) {
                                        continue;
                                    }
                                    if (!((index >= layer && index <= max) || (index <= layer && index >= min))) {
                                        continue;
                                    }
                                    if (layerState(nodePool[j]) != "active")
                                        node->set("state", "hover");
                                }
                                return;
                            }
                        } else if ((keys.count("LCTRL") || keys.count("RCTRL")) && getSelectionRange().second != -1) {
                            if (auto state = layerState(item); state != "hover" && state != "active") {
                                item->set("state", "hover");
                            } else if (state == "hover"){
                                item->set("state", "enabled");
                            }
                            return;
                        }
                        if (inject<Command> cmd{"activatelayer"}; cmd) {
                            cmd->set("layer", layer);
                            cmd->run();
                        }
                    })},
                    {"state", layer == currentLayer ? "active" : "enabled"},
                    {"cell", cell},
                });

            if (!item->getParent())
                node()->addChild(item);

            height += itemHeight;
        }
    }

    String layerState(std::shared_ptr<ui::Node> node) {
        auto prop = node->get("state");
        return prop ? prop->get<String>() : "";
    }

    S32 layerNumber(std::shared_ptr<ui::Node> node) {
        if (!node->getParent())
            return -1;
        auto prop = node->get("layer-number");
        return prop ? prop->get<S32>() : -1;
    }

    std::pair<S32, S32> getSelectionRange() {
        S32 min{std::numeric_limits<S32>::max()};
        S32 max = -1;
        for (S32 i = 0, size = nodePool.size(); i < size; ++i) {
            auto node = nodePool[i];
            if (!node->getParent())
                continue;
            auto state = layerState(node);
            if (state == "enabled")
                continue;
            auto num = layerNumber(node);
            min = std::min(min, num);
            max = std::max(max, num);
        }
        return {min, max};
    }
};

static ui::Controller::Shared<LayerList> layerlist{"layerlist"};
