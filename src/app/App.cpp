// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <chrono>
#include <thread>

#include <app/App.hpp>
#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/PropertySet.hpp>
#include <common/System.hpp>
#include <fs/Cache.hpp>
#include <fs/FileSystem.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class AppImpl : public App {
public:
    bool running = true;
    inject<Log> log;
    inject<System> system;

    inject<Cache> cache{"new"};
    Cache::Provides globalCache{cache.get()};

    inject<FileSystem> fs{"new"};
    FileSystem::Provides globalFS{fs.get()};

    inject<Config> config{"new"};
    Config::Provides globalConfig{config.get()};

    PubSub<msg::Shutdown> pub{this};

    using clock = std::chrono::high_resolution_clock;

    clock::time_point referenceTime;

    void boot(int argc, const char* argv[]) override {
        referenceTime = clock::now();
        log->setGlobal();
        log->setLevel(Log::Level::VERBOSE); // TODO: Configure level using args
        fs->boot();
        config->boot();
        system->boot();
        Tool::boot();
        ui::Node::fromXML("MainWindow");
        pub(msg::BootComplete{});
    }

    bool run() override {
        if (!system->run())
            return false;
        clock::time_point now = clock::now();
        auto delta = now - referenceTime;
        referenceTime = now;
        std::this_thread::sleep_for(std::chrono::milliseconds{1000 / 60} - delta);
        return running;
    }

    void on(msg::Shutdown&) {
        logI("Shutting down");
    }

    ~AppImpl() {
        pub(msg::Shutdown{});
    }
};

static App::Shared<AppImpl> app{"dotto"};
