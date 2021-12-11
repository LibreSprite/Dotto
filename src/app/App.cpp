// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <app/App.hpp>
#include <common/System.hpp>
#include <fs/FileSystem.hpp>
#include <log/Log.hpp>

class AppImpl : public App {
public:
    bool running = true;
    inject<Log> log;
    inject<System> system;
    inject<FileSystem> fs;

    void boot(int argc, const char* argv[]) override {
        bootLogger();
        bootFS();
        system->boot();
        Log::write(Log::Level::VERBOSE, "Booting complete");
    }

    bool run() override {
        return running;
    }

    void bootLogger() {
        log->setGlobal();
        log->setLevel(Log::Level::VERBOSE); // TODO: Configure level using args
    }

    void bootFS() {

    }
};

static App::Shared<AppImpl> app{"dotto"};
