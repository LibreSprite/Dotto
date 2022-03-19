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
#include <fs/Folder.hpp>
#include <log/Log.hpp>
#include <script/Engine.hpp>

using namespace fs;

class AppImpl : public App {
public:
    bool running = true;
    inject<Log> log;
    inject<System> system{"new"};
    System::Provides globalSystem{system.get()};

    inject<Cache> cache{"new"};
    Cache::Provides globalCache{cache.get()};

    inject<FileSystem> fs{"new"};
    FileSystem::Provides globalFS{fs.get()};

    inject<Config> config{"new"};
    Config::Provides globalConfig{config.get()};

    PubSub<msg::RequestShutdown> pub{this};

    using clock = std::chrono::high_resolution_clock;

    clock::time_point referenceTime;

    void boot(int argc, const char* argv[]) override {
        auto lock = cache->lock();
        referenceTime = clock::now();
        log->setGlobal();
#ifdef _DEBUG
        log->setLevel(Log::Level::VERBOSE); // TODO: Configure level using args
#else
        log->setLevel(Log::Level::INFO); // TODO: Configure level using args
#endif
        fs->boot();
        config->boot();
        system->boot();
        if (auto autorun = fs->find("%appdata/autorun", "dir")->get<Folder>()) {
            Vector<std::pair<S32, std::shared_ptr<File>>> files;
            autorun->forEach([&](std::shared_ptr<FSEntity> child) {
                if (child->isFile()) {
                    auto file = child->get<File>();
                    S32 priority = atoi(file->name().c_str());
                    if (priority)
                        files.emplace_back(priority, file);
                }
            });
            std::sort(files.begin(), files.end(), [](auto& left, auto& right) {
                return left.first < right.first;
            });
            for (auto& entry : files)
                entry.second->parse();
        }
        pub(msg::BootComplete{});
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - referenceTime);
        logI("Boot time: ", delta.count());
    }

    bool run() override {
        pub(msg::Tick{});
        pub(msg::PostTick{});

#ifndef EMSCRIPTEN
        if (running) {
            clock::time_point now = clock::now();
            auto delta = now - referenceTime;
            referenceTime = now;
            std::this_thread::sleep_for(std::chrono::milliseconds{1000 / 60} - delta);
        }
#endif

        return running;
    }

    void on(msg::RequestShutdown&) {
        running = false;
    }

    ~AppImpl() {
        pub(msg::Shutdown{});
    }
};

static App::Shared<AppImpl> app{"dotto"};
