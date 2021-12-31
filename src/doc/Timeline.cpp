// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Cell.hpp>
#include <doc/Timeline.hpp>

class TimelineImpl : public Timeline {
public:
    Vector<Vector<std::shared_ptr<Cell>>> data;

    U32 frameCount() const override {
        return data.size();
    }

    U32 layerCount() const override {
        if (data.empty()) return 0;
        return data[0].size();
    }

    void setFrameCount(U32 count) {
        data.resize(count);
    }

    void setLayerCount(U32 count) {
        for (auto& layer : data) {
            layer.resize(count);
        }
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
        return data[frame][layer];
    }

    void setCell(U32 frame, U32 layer, std::shared_ptr<Cell> cell) override {
        if (frame >= frameCount())
            setFrameCount(frame + 1);
        if (layer >= layerCount())
            setLayerCount(layer + 1);
        data[frame][layer] = cell;
    }
};

static Timeline::Shared<TimelineImpl> reg{"new"};
