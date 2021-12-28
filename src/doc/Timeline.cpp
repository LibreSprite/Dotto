// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Cell.hpp>
#include <doc/Timeline.hpp>

class TimelineImpl : public Timeline {
public:
    Vector<Vector<std::shared_ptr<Cell>>> data;

    U32 frameCount() override {
        return data.size();
    }

    U32 layerCount() override {
        if (data.empty()) return 0;
        return data[0].size();
    }

    std::shared_ptr<Cell> getCell(U32 frame, U32 layer, bool loop) override {
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
};

static Timeline::Shared<TimelineImpl> reg{"new"};
