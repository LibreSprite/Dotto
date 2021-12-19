// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>
#include <common/Value.hpp>
#include <fs/File.hpp>

class StdFile : public File {
public:
    FILE* file = nullptr;
    String path;

    void init(const Value& resource) override {
        if (resource.has<String>())
            path = resource.get<String>();
    }

    String getUID() override {return path;}

    bool open(const FileOpenSettings& settings) override {
        close();
        const char* mode = settings.write ? "rb+" : "rb";
        file = fopen(path.c_str(), mode);
        if (!file && settings.create && settings.write)
            file = fopen(path.c_str(), "wb+");
        return file;
    }

    ~StdFile() {
        close();
    }

    void close() override {
        if (file) {
            fclose(file);
        }
        file = nullptr;
    }

    String type() override {
        return inject<FileSystem>{}->extension(path);
    }

    bool isOpen() override {
        return file;
    }

    U64 size() override {
        if (!file)
            return 0;
        auto pos = ftell(file);
        fseek(file, 0, SEEK_END);
        auto end = ftell(file);
        fseek(file, pos, SEEK_SET);
        return end;
    }

    bool seek(U64 position) override {
        if (!file)
            return false;
        fseek(file, position, SEEK_SET);
        return true;
    }

    U64 tell() override {
        return file ? ftell(file) : 0;
    }

    U64 read(void* buffer, U64 size) override {
        if (!file)
            return 0;
        return fread(buffer, size, 1, file);
    }

    U64 write(const void* buffer, U64 size) override {
        if (!file)
            return 0;
        return fwrite(buffer, size, 1, file);
    }
};
