// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef __APPLE__

#include <array>
#include <limits.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include <Foundation/Foundation.h>

#include <common/String.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <fs/StdFile.hpp>
#include <sys/types.h>
#include <sys/stat.h>

static auto fontPaths = std::array{
    "/System/Library/Fonts/",
    "/Library/Fonts",
    "~/Library/Fonts"
};

class MacOSRootDir : public RootFolder {
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

            NSArray* dirs = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
            if (dirs) {
                NSString* dir = [dirs firstObject];
                if (dir)
                    config = [dir UTF8String];
            }

            if (config.empty())
                config = home + separator + ".config";
            config += separator + "dotto";
            mount("%userdata", "dir", config);
        }

        {
            String exePath;
            exePath.resize(MAXPATHLEN);
            uint32_t size = exePath.size();
            while (_NSGetExecutablePath(&exePath[0], &size) == -1)
                exePath.resize(size);
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

static FSEntity::Shared<MacOSRootDir> lrd{"rootDir"};
static FileSystem::Shared<FileSystem> fs{"new"};
static File::Shared<StdFile> stdFile{"std"};
static File::Shared<Folder> stdDir{"dir"};

#endif
