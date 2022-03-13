// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/inject.hpp>
#include <gui/Window.hpp>

class System : public Injectable<System> {
public:
    PubSub<msg::PostTick> pub{this};

    void on(msg::PostTick&){
        if (!run())
            pub(msg::RequestShutdown{});
    }

    virtual void setMouseCursorVisible(bool) = 0;
    virtual bool boot() = 0;
    virtual bool run() = 0;
};
