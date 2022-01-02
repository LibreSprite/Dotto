// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class ZoomCommand : public Command {
    Property<String> level{this, "level"};
    PubSub<> pub{this};

public:
    void run() override {
        auto editor = pub(msg::PollActiveEditor{}).editor;
        if (!editor)
            return;
        auto& level = *this->level;
        if (level.empty())
            return;

        auto scalevalue = editor->get("scale");
        F32 value = 1.0f;
        if (scalevalue) {
            if (scalevalue->has<F32>())
                value = scalevalue->get<F32>();
            else if (scalevalue->has<S32>())
                value = scalevalue->get<S32>();
            else if (scalevalue->has<String>())
                value = atoi(scalevalue->get<String>().c_str());
        }


        if (level[0] == '*') {
            value *= atof(level.c_str() + 1);
        } else if (level[0] == '+') {
            value += atof(level.c_str() + 1);
        } else if (level[0] == '-') {
            value += atof(level.c_str());
        } else {
            value = atof(level.c_str()) / 100.0f;
        }

        logV("Setting zoom level to ", value);
        editor->set("scale", value);
    }
};

static Command::Shared<ZoomCommand> cmd{"zoom"};
