// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Config.hpp>
#include <common/Parser.hpp>
#include <fs/FileSystem.hpp>

class ConfigImpl : public Config {
public:
    inject<FileSystem> fs;

    bool boot() override {
        auto file = fs->find("%userdata/settings.ini")->get<File>();
        if (!file || !file->open()) {
            file = fs->find("%appdata/settings.ini")->get<File>();
        }
        if (!file || !file->open()) {
            Log::write(Log::Level::ERROR, "Could not open settings file. Reinstalling may fix this problem.");
        } else if (auto parser = inject<Parser>{"ini"}) {
            properties = parser->parseFile(file).get<std::shared_ptr<PropertySet>>();
        }
        return true;
    }
};

static Config::Shared<ConfigImpl> cfg{"new"};
