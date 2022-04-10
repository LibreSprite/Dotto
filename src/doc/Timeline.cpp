// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Cell.hpp>
#include <doc/Selection.hpp>
#include <doc/Timeline.hpp>

class TimelineImpl : public Timeline {
public:
    Vector<Vector<std::shared_ptr<Cell>>> data;
    std::shared_ptr<Selection> selection;
    U32 _frame = 0, _layer = 0;

    U32 frameCount() const override {
        return data.size();
    }

    U32 layerCount() const override {
        if (data.empty()) return 0;
        return data[0].size();
    }

    void setFrameCount(U32 count) {
        logI("Set frame count: ", count);
        data.resize(count);
    }

    void setLayerCount(U32 count) {
        logI("Set layer count: ", count);
        for (auto& layer : data) {
            layer.resize(count);
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

    std::shared_ptr<Cell> getCell(U32 frame, U32 layer, bool loop) const override {
        if (data.empty())
            return nullptr;
        if (loop) {
            frame %= frameCount();
            if (data[frame].empty())
                return nullptr;
            loop %= data[frame].size();
        } else {
            if (frame >= data.size())
                return nullptr;
            if (loop >= data[frame].size())
                return nullptr;
        }
        if (layer >= data[frame].size())
            return nullptr;
        return data[frame][layer];
    }

    void setCell(U32 frame, U32 layer, std::shared_ptr<Cell> cell) override {
        if (!cell) {
            if (frame >= frameCount() || layer >= layerCount())
                return;
        }
        if (frame >= frameCount())
            setFrameCount(frame + 1);
        if (layer >= layerCount())
            setLayerCount(layer + 1);
        data[frame][layer] = cell;
    }

    void setSelection(const Selection* selection) {
        if (!selection) {
            this->selection.reset();
        } else {
            if (!this->selection) {
                this->selection = inject<Selection>{"new"};
            }
            *this->selection = *selection;
        }
    }

    Selection* getSelection() {
        return selection.get();
    }
};

static Timeline::Shared<TimelineImpl> reg{"new"};
