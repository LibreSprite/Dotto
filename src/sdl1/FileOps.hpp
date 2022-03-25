// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <fs/File.hpp>

using namespace fs;

class FileOps : public SDL_RWops {
public:
    std::shared_ptr<File> file;

    FileOps(std::shared_ptr<File> file) : file{file} {
        seek = _seek;
        read = _read;
        write = _write;
        close = _close;
    }

    static File& f(SDL_RWops* ctx) {
        return *static_cast<FileOps*>(ctx)->file;
    }

    static int _seek(struct SDL_RWops* context, int offset, int whence) {
        auto& file = f(context);
        if (whence == RW_SEEK_SET) {
            file.seek(offset);
        } else if (whence == RW_SEEK_CUR) {
            file.seek(offset + file.tell());
        } else if (whence == RW_SEEK_END) {
            file.seek(file.tell() - offset);
        } else {
            return -1;
        }
        return file.tell();
    }

    static int _read(struct SDL_RWops * context, void *ptr, int size, int maxnum) {
        return f(context).read(ptr, size * maxnum) / size;
    }

    static int _write(struct SDL_RWops * context, const void *ptr, int size, int num) {
        return f(context).write(ptr, size * num) / size;
    }

    static int _close(struct SDL_RWops * context) {
        return 0;
    }
};
