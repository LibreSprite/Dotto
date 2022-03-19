// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef EMSCRIPTEN

#include <limits.h>
#include <unistd.h>

#include <common/String.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <fs/StdFile.hpp>

using namespace fs;

class WasmRootDir : public RootFolder {
public:
    bool boot() {
        String appdir, home, config;
        separator = "/";

        {
            home = "/";
            if (home.empty())
                home = "~";
            mount("%userhome", "dir", home);
        }

        {
            config = "/";
            if (config.empty())
                config = home + separator + ".config";
            config += separator + "dotto";
            mount("%userdata", "dir", config);
        }

        {
            appdir = "/";
            mount("%appdir", "dir", appdir);
            mount("%appdata", "dir", appdir + separator + "data");
        }

        {
            mount("%fonts", "dir", "");
            if (auto fonts = std::static_pointer_cast<Folder>(getChild("%fonts", ""))) {
                fonts->mount("0", "dir", String("/fonts"));
            }
        }
        return true;
    }
};

static FSEntity::Shared<WasmRootDir> lrd{"rootDir"};
static FileSystem::Shared<FileSystem> reg{"new"};
static File::Shared<StdFile> stdFile{"std"};
static File::Shared<Folder> stdDir{"dir"};

#endif
