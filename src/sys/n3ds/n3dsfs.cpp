// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef __N3DS__

#include <common/String.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <fs/StdFile.hpp>

using namespace fs;

class N3DSRootDir : public RootFolder {
public:
    bool boot() {
        String appdir, home, config;
        separator = "/";

        {
            home = "/dotto";
            mount("%userhome", "dir", home);
        }

        {
            config = "/dotto/data";
            mount("%userdata", "dir", config);
        }

        {
            appdir = "/dotto";
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

static FSEntity::Shared<N3DSRootDir> lrd{"rootDir"};
static FileSystem::Shared<FileSystem> reg{"new"};
static File::Shared<StdFile> stdFile{"std"};
static File::Shared<Folder> stdDir{"dir"};

#endif
