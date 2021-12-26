// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class MainWindow : public ui::Controller {
public:
    void attach() override {
        Tool::boot();
    }
};

static ui::Controller::Shared<MainWindow> mainwindow{"main"};
