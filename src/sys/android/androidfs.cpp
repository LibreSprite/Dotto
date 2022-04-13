// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(__ANDROID__)

#include <SDL.h>

#include <limits.h>
#include <unistd.h>

#include <common/String.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>
#include <fs/File.hpp>
#include <sys/types.h>
#include <sys/stat.h>

using namespace fs;

class AndroidRootDir : public RootFolder {
public:
    bool boot() {
        String appdir, home;
        separator = "/";
        home = SDL_AndroidGetExternalStoragePath();
        appdir = SDL_AndroidGetInternalStoragePath();
        mount("%userhome", "dir", home);
        mount("%userdata", "dir", home);
        mount("%appdir", "dir", appdir);
        mount("%appdata", "dir", "appdata:");

        {
            mount("%fonts", "dir", "");
            // if (auto fonts = std::static_pointer_cast<Folder>(getChild("%fonts", ""))) {
            //     U32 index = 0;
            //     for (auto& path : fontPaths) {
            //         struct stat buf;
            //         if (::stat(path, &buf) == 0 && S_ISDIR(buf.st_mode)) {
            //             fonts->mount(std::to_string(index++), "dir", String(path));
            //         }
            //     }
            // }
        }
        return true;
    }
};

class AndroidFile : public File {
public:
    SDL_RWops* file = nullptr;
    String path;

    void init(const Value& resource) override {
        if (resource.has<String>())
            path = resource.get<String>();
    }

    String getUID() override {return path;}

    bool open(const FileOpenSettings& settings) override {
        close();
        const char* mode = settings.write ? "rb+" : "rb";
        auto path = this->path.c_str();
        bool isAppData = startsWith(this->path, "appdata:");
        if (isAppData)
            path += 9;
        file = SDL_RWFromFile(path, mode);
        if (!file && settings.create && settings.write)
            file = SDL_RWFromFile(path, "wb+");
        return file;
    }

    ~AndroidFile() {
        close();
    }

    void close() override {
        if (file) {
            SDL_RWclose(file);
        }
        file = nullptr;
    }

    String type() override {
        return inject<FileSystem>{}->extension(path);
    }

    String name() override {
        return split(path, separator).back();
    }

    bool isOpen() override {
        return file;
    }

    U64 size() override {
        return file ? SDL_RWsize(file) : 0;
    }

    bool seek(U64 position) override {
        if (!file)
            return false;
        SDL_RWseek(file, position, RW_SEEK_SET);
        return true;
    }

    U64 tell() override {
        return file ? SDL_RWtell(file) : 0;
    }

    U64 read(void* buffer, U64 size) override {
        return file ? SDL_RWread(file, buffer, 1, size) : 0;
    }

    U64 write(const void* buffer, U64 size) override {
        return file ? SDL_RWwrite(file, buffer, size, 1) : 0;
    }
};

static FSEntity::Shared<AndroidRootDir> lrd{"rootDir"};
static FileSystem::Shared<FileSystem> reg{"new"};
static File::Shared<AndroidFile> stdFile{"std"};
static File::Shared<Folder> stdDir{"dir"};

#endif
