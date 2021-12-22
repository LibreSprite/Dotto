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
        properties = fs->parse("%userdata/settings.ini");
        if (!properties)
            properties = fs->parse("%appdata/settings.ini");
        if (!properties) {
            Log::write(Log::Level::ERROR, "Could not open settings file. Reinstalling may fix this problem.");
            return false;
        }
        return true;
    }
};

static Config::Shared<ConfigImpl> cfg{"new"};
