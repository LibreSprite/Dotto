// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <chrono>

#include <common/String.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>

class DocumentImpl : public Document {
    HashMap<String, std::shared_ptr<Timeline>> guidToTimeline;

public:
    String getGUID() {
        U32 now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        static U32 counter = now * U32(rand());
        counter++;
        return tostring(counter, 36) + "-" + tostring(now, 36);
    }

    std::shared_ptr<Timeline> createTimeline() override {
        Provides doc{this};
        auto timeline = inject<Timeline>{"new"}.shared();
        timeline->GUID = getGUID();
        guidToTimeline[timeline->GUID] = timeline;
        return timeline;
    }

    const HashMap<String, std::shared_ptr<Timeline>>& getTimelines() override {
        return guidToTimeline;
    }
};

static Document::Shared<DocumentImpl> reg{"new"};
