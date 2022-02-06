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

class ActivateLayer : public Command {
    inject<ui::Node> editor{"activeeditor"};
    Property<U32> layer{this, "layer", ~U32{}};
    Property<S32> navigate{this, "navigate", 0};
    U32 prevLayer = -1;

public:
    void undo() override {
        editor->set("layer", prevLayer);
    }

    void run() override {
        auto timeline = doc()->currentTimeline();
        if (!timeline)
            return;
        auto& ps = editor->getPropertySet();
        if (prevLayer == -1)
            prevLayer = ps.get<U32>("layer");
        auto layer = *this->layer;
        if (layer >= timeline->layerCount())
            layer = ~U32{};
        if (layer == ~U32{})
            layer = prevLayer + navigate;
        if (layer != prevLayer) {
            editor->set("layer", layer);
            commit();
        }
    }
};

static Command::Shared<ActivateLayer> cmd{"activatelayer"};
