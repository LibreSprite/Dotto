// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef __WINDOWS__

#include <windows.h>
#include <codecvt>
#include <shlobj.h>

#undef ERROR

#include <common/String.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <fs/StdFile.hpp>
#include <sys/types.h>
#include <sys/stat.h>

class WindowsRootDir : public fs::RootFolder {
public:
    bool boot() {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
        String osdir = std::getenv("windir");

        String appdir, home, config;
        separator = "\\";

        {
            TCHAR homebuf[MAX_PATH+1] = {0};
            SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, homebuf);
            home = homebuf;
            logI("Home: ", home);
            mount("%userhome", "dir", home);
        }

        {
            config = std::getenv("AppData") ?: "";
            if (config.empty())
                config = home;
            config += separator + "dotto";
            logI("UserData: ", home);
            mount("%userdata", "dir", config);
        }

        {
            TCHAR exePath[MAX_PATH+1] = {0};
            ::GetModuleFileName(NULL, exePath, sizeof(exePath)/sizeof(TCHAR));
            auto parts = split(exePath, separator);
            parts.pop_back();
            appdir = join(parts, separator);
            logI("AppDir: ", appdir);
            mount("%appdir", "dir", appdir);
            mount("%appdata", "dir", appdir + separator + "data");
        }

        {
            mount("%fonts", "dir", "");
            if (auto fonts = std::static_pointer_cast<Folder>(getChild("%fonts", ""))) {
                auto fontdir = osdir + separator + "fonts";
                logI("Fonts:", fontdir);
                fonts->mount("0", "dir", fontdir);
            }
        }

        return true;
    }
};

static FSEntity::Shared<WindowsRootDir> lrd{"rootDir"};
static FileSystem::Shared<FileSystem> fsreg{"new"};
static File::Shared<StdFile> stdFile{"std"};
static File::Shared<fs::Folder> stdDir{"dir"};

#endif
