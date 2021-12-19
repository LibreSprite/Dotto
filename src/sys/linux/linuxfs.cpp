// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef __linux__

#include <limits.h>
#include <unistd.h>

#include <common/String.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <fs/StdFile.hpp>
#include <sys/types.h>
#include <sys/stat.h>

static auto fontPaths = std::array{
    "/usr/share/fonts",
    "/usr/local/share/fonts",
    "~/.local/share/fonts",
    "~/.fonts",
    "/usr/share/fonts/otf",
    "/usr/local/share/fonts/otf",
    "~/.local/share/fonts/otf",
    "/usr/share/fonts/OTF",
    "/usr/local/share/fonts/OTF",
    "~/.local/share/fonts/OTF",
    "~/.font/OTF",
    "/usr/share/fonts/TTF",
    "/usr/local/share/fonts/TTF",
    "~/.local/share/fonts/TTF",
    "~/.font/TTF",
    "/usr/share/fonts/ttf",
    "/usr/local/share/fonts/ttf",
    "~/.local/share/fonts/ttf",
    "~/.font/ttf"
};

class LinuxRootDir : public RootFolder {
public:
    bool boot() {
        String appdir, home, config;
        separator = "/";

        {
            home = std::getenv("HOME") ?: "";
            if (home.empty())
                home = "~";
            mount("%userhome", "dir", home);
        }

        {
            config = std::getenv("XDG_CONFIG_HOME") ?: "";
            if (config.empty())
                config = home + separator + ".config";
            config += separator + "dotto";
            mount("%userdata", "dir", config);
        }

        {
            char exePath[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath));
            if (len == -1 || len == sizeof(exePath))
                len = 0;
            exePath[len] = '\0';
            auto parts = split(exePath, separator);
            parts.pop_back();
            appdir = join(parts, separator);
            mount("%appdir", "dir", appdir);
            mount("%appdata", "dir", appdir + separator + "data");
        }

        {
            mount("%fonts", "dir", "");
            if (auto fonts = std::static_pointer_cast<Folder>(getChild("%fonts", ""))) {
                U32 index = 0;
                for (auto& path : fontPaths) {
                    struct stat buf;
                    if (::stat(path, &buf) == 0 && S_ISDIR(buf.st_mode)) {
                        fonts->mount(std::to_string(index++), "dir", String(path));
                    }
                }
            }
        }
        return true;
    }
};

static FSEntity::Shared<LinuxRootDir> lrd{"rootDir"};
static FileSystem::Shared<FileSystem> fs{"new"};
static File::Shared<StdFile> stdFile{"std"};
static File::Shared<Folder> stdDir{"dir"};

#endif
