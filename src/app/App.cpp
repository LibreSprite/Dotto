// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <app/App.hpp>
#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/PropertySet.hpp>
#include <common/System.hpp>
#include <fs/FileSystem.hpp>
#include <log/Log.hpp>

class AppImpl : public App {
public:
    bool running = true;
    inject<Log> log;
    inject<System> system;

    inject<FileSystem> fs{"new"};
    FileSystem::Provides globalFS{fs.get()};

    inject<Config> config{"new"};
    Config::Provides globalConfig{config.get()};

    PubSub<msg::Shutdown> pub{this};

    void boot(int argc, const char* argv[]) override {
        log->setGlobal();
        log->setLevel(Log::Level::VERBOSE); // TODO: Configure level using args
        fs->boot();
        config->boot();
        system->boot();
        openMainWindow();
        pub(msg::BootComplete{});
    }

    void openMainWindow() {
        PropertySet main;
        main.append(config->properties->get<std::shared_ptr<PropertySet>>("Window"));
        main.append(config->properties->get<std::shared_ptr<PropertySet>>("MainWindow"));
        system->openWindow(main);
    }

    bool run() override {
        if (!system->run())
            return false;
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
