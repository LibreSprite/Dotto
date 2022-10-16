// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <filesystem>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <gui/Controller.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <optional>
#include <task/TaskManager.hpp>

namespace msg {
struct RecorderStart {};
struct RecorderStop {};
struct RecorderUpdateSequence {U32 number;};
struct RecorderEncodingStart {};
struct RecorderEncodingDone {};
struct RecorderEncodingFail {};
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

class Encoder {
public:
    TaskHandle handle;

    bool done() {
        return handle.task->isDone();
    }

    void run() {
        String cmd;
        PubSub<>::pub(msg::RecorderEncodingStart{});

        auto userdata = std::filesystem::path{inject<FileSystem>{}->find("%userdata", "dir")->getUID()};
        cmd += "cd " + join(split(userdata.string(), "\\"), "\\\\") + " && ";

        cmd += "\"";
        if (auto ffmpeg = inject<Config>{}->properties->get<String>("ffmpeg-path"); ffmpeg.empty()) {
            cmd += "ffmpeg";
        } else {
            cmd += join(split(ffmpeg, "\\"), "\\\\");
        }
        cmd += "\" -y ";
        cmd += "-start_number 1 -i \"recording_%d.png\" -c:v libx264 -r 30 -pix_fmt yuv420p out.mp4";

        logI(cmd);

        handle = inject<TaskManager>{}->add(
            [=]() -> int {
                return system(cmd.c_str());
            },
            [=](int code){
                if (code == 0) {
                    PubSub<>::pub(msg::RecorderEncodingDone{});
                } else {
                    PubSub<>::pub(msg::RecorderEncodingFail{});
                }
            });
    }
};
static std::optional<Encoder> encoder;

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

class RecorderEncode : public ui::Controller {
public:
    PubSub<msg::RecorderEncodingStart,
           msg::RecorderEncodingDone,
           msg::RecorderEncodingFail,
           msg::RecorderUpdateSequence> pub{this};
    Property<String> label{this, "label", "${value}"};

    void attach() override {
        if (encoder) {
            if (encoder->done())
                on(msg::RecorderEncodingDone{});
            else
                on(msg::RecorderEncodingStart{});
        } else {
            on(msg::RecorderUpdateSequence{});
        }
    }

    void on(const msg::RecorderUpdateSequence&) {
        node()->set("text", "Encode MP4");
    }

    void on(const msg::RecorderEncodingDone&) {
        node()->set("text", "Encoding Done");
    }

    void on(const msg::RecorderEncodingStart&) {
        node()->set("text", "Encoding MP4");
    }

    void on(const msg::RecorderEncodingFail&) {
        node()->set("text", "Encoding Error");
    }
};
static ui::Controller::Shared<RecorderEncode> recenc{"recorderencode"};

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

class RecorderEncodeCmd : public Command {
public:
    void run() override {
        if (encoder && !encoder->done()) {
            return;
        }
        encoder = std::nullopt;
        encoder.emplace();
        encoder->run();
    }
};
static Command::Shared<RecorderEncodeCmd> cmd3{"recorderencode"};
