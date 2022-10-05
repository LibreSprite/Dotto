// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <fs/FileSystem.hpp>
#include <log/Log.hpp>

class Recorder {
    PubSub<msg::ModifyDocument,
           msg::Tick> pub{this};
    U32 sequence = 0;
    bool tick = true;
public:
    void on(msg::Tick&) {
        tick = true;
    }

    void on(msg::ModifyDocument& event) {
        auto doc = event.doc;
        if (!doc)
            return;
        auto timeline = doc->currentTimeline();
        if (!timeline)
            return;
        auto group = timeline->getCell(timeline->frame());
        if (!group)
            return;
        auto surface = group->getComposite();
        if (!surface)
            return;
        auto shared = surface->shared_from_this();
        FileSystem::write("%userdata/recording_" + std::to_string(sequence) + ".png", shared);
        if (tick) {
            tick = false;
            sequence++;
        }
    }
};

static std::optional<Recorder> recorder;

class ToggleRecording : public Command {
public:
    void run() override {
        if (recorder) {
            recorder = std::nullopt;
        } else {
            recorder.emplace();
        }
    }
};


static Command::Shared<ToggleRecording> cmd{"togglerec"};
