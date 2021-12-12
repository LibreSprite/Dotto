// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <app/App.hpp>
#include <common/PropertySet.hpp>
#include <common/System.hpp>
#include <common/Config.hpp>
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

    void boot(int argc, const char* argv[]) override {
        log->setGlobal();
        log->setLevel(Log::Level::VERBOSE); // TODO: Configure level using args
        fs->boot();
        config->boot();
        system->boot();
        openMainWindow();
    }

    void openMainWindow() {
        PropertySet main;
        main.append(config->properties->get<std::shared_ptr<PropertySet>>("GfxMode"));
        system->openWindow(main);
    }

    bool run() override {
        if (!system->run())
            return false;
        return running;
    }
};

static App::Shared<AppImpl> app{"dotto"};
