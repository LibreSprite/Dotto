// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <filesystem>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <gui/Controller.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>

namespace msg {
struct RecorderStart {};
struct RecorderStop {};
struct RecorderUpdateSequence {U32 number;};
}

U32 initSequence() {
    inject<FileSystem> fs;
    U32 highest = 0;
    if (auto userdata = fs->find("%userdata", "dir")->get<fs::Folder>()) {
        std::regex expr{"recording_([0-9]+)\\.png"};
        userdata->forEach([&](std::shared_ptr<fs::FSEntity> child) {
            auto name = std::filesystem::path{child->getUID()}.filename().string();
            std::cmatch match;
            std::regex_match(name.c_str(), match, expr);
            if (!match.empty()) {
                highest = std::max<U32>(highest, std::atoi(match[1].str().c_str()));
            }
        });
    }
    return highest + 1;
}

U32& sequence() {
    static U32 num = initSequence();
    return num;
}

class Recorder {
    PubSub<msg::ModifyDocument,
           msg::Tick> pub{this};
    bool tick = true;
public:
    Recorder() {
        pub(msg::RecorderStart{});
    }

    ~Recorder() {
        pub(msg::RecorderStop{});
    }

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
        FileSystem::write("%userdata/recording_" + std::to_string(sequence()) + ".png", shared);
        if (tick) {
            tick = false;
            pub(msg::RecorderUpdateSequence{++sequence()});
        }
    }
};

static std::optional<Recorder> recorder;

class RecorderStatus : public ui::Controller {
public:
    PubSub<msg::RecorderStart, msg::RecorderStop> pub{this};
    Property<String> recording{this, "recording", "Recording"};
    Property<String> stopped{this, "stopped", "Stopped"};
    void on(const msg::RecorderStart&) {node()->load({{"text", *recording}});}
    void on(const msg::RecorderStop&) {node()->load({{"text", *stopped}});}
    void attach() override {
        if (recorder) {
            on(msg::RecorderStart{});
        } else {
            on(msg::RecorderStop{});
        }
    }
};
static ui::Controller::Shared<RecorderStatus> recstat{"recorderstatus"};

class RecorderSequence : public ui::Controller {
public:
    PubSub<msg::RecorderUpdateSequence> pub{this};
    Property<String> label{this, "label", "${value}"};

    void attach() override {
        on(msg::RecorderUpdateSequence{sequence()});
    }

    void on(const msg::RecorderUpdateSequence& event) {
        node()->set("value", event.number);
        if (!label->empty()) {
            node()->load({{"text", ""}}); // force text redraw
            node()->load({{"text", *label}});
        }
    }
};
static ui::Controller::Shared<RecorderSequence> recseq{"recordersequence"};

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

class ResetRecordingSequence : public Command {
public:
    void run() override {
        sequence() = 1;
        PubSub<>::pub(msg::RecorderUpdateSequence{1});
    }
};
static Command::Shared<ResetRecordingSequence> cmd2{"resetrecsequence"};
