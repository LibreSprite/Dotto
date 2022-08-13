// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/GroupCell.hpp>
#include <doc/Selection.hpp>
#include <doc/Timeline.hpp>

class TimelineImpl : public Timeline {
public:
    Document* doc = nullptr;
    Vector<std::shared_ptr<GroupCell>> data;
    U32 _frame = 0, _layer = 0;

    void setDocument(Document* document) override {
        doc = document;
    }

    Document* document() const override {
        return doc;
    }

    U32 frameCount() const override {
        return data.size();
    }

    U32 layerCount() const override {
        if (data.empty()) return 0;
        return data[0]->layerCount();
    }

    void setFrameCount(U32 count) {
        logI("Set frame count: ", count);
        data.resize(count);
        for (auto& cell : data) {
            if (!cell)
                cell = std::static_pointer_cast<GroupCell>(inject<Cell>{"group"}.shared());
        }
    }

    void setLayerCount(U32 count) {
        logI("Set layer count: ", count);
        for (auto& layer : data) {
            layer->resize(count);
        }
    }

    U32 frame() const override {return _frame;}
    U32 layer() const override {return _layer;}

    std::shared_ptr<Cell> activate(U32 frame, U32 layer) override {
        auto cell = getCell(frame, layer, false);
        if (cell) {
            _frame = frame;
            _layer = layer;
        }
        return cell;
    }

    std::shared_ptr<Cell> getCell(U32 frame) const override {
        return (frame >= data.size()) ? nullptr : data[frame];
    }

    std::shared_ptr<Cell> getCell(U32 frame, U32 layer, bool loop) const override {
        if (data.empty())
            return nullptr;
        if (loop) {
            frame %= frameCount();
            layer %= data[frame]->layerCount();
        } else {
            if (frame >= data.size())
                return nullptr;
        }
        return data[frame]->getCell(layer);
    }

    void setCell(U32 frame, U32 layer, std::shared_ptr<Cell> cell) override {
        if (!cell && frame >= frameCount()) {
            return;
        }
        if (frame >= frameCount())
            setFrameCount(frame + 1);
        data[frame]->setCell(layer, cell);
    }
};

static Timeline::Shared<TimelineImpl> reg{"new"};
