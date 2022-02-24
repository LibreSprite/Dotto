// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <chrono>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>

class FPS : public ui::Controller {
public:
    PubSub<msg::Tick> pub{this};

    using clock = std::chrono::high_resolution_clock;
    clock::time_point referenceTime = clock::now();
    U32 frames = 0;
    
    void on(msg::Tick&) {
        auto now = clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - referenceTime);
        if (delta.count() >= 1000) {
            node()->set("text", frames);
            referenceTime = now;
            frames = 0;
        }
        frames++;
    }
};

static ui::Controller::Shared<FPS> fps{"fps"};
