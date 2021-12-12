// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "String.hpp"
#include <unistd.h>
#include <limits.h>

#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <fs/StdFile.hpp>

class LinuxRootDir : public Folder {
public:
    LinuxRootDir() {
        separator = "/";

        String home = std::getenv("HOME") ?: "";
        if (home.empty())
            home = "~";
        mount("%userhome", "dir", home);

        String config = std::getenv("XDG_CONFIG_HOME") ?: "";
        if (config.empty())
            config = home + separator + ".config";
        config += separator + "dotto";

        mount("%userdata", "dir", config);


        char exePath[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath));
        if (len == -1 || len == sizeof(exePath))
            len = 0;
        exePath[len] = '\0';
        auto parts = split(exePath, separator);
        parts.pop_back();
        auto appdir = join(parts, separator);
        mount("%appdir", "dir", appdir);

        mount("%appdata", "dir", appdir + separator + "data");
    }
};

static FSEntity::Shared<LinuxRootDir> lrd{"rootDir"};
static FileSystem::Shared<FileSystem> fs{"new"};
static File::Shared<StdFile> stdFile{"std"};
static File::Shared<Folder> stdDir{"dir"};
