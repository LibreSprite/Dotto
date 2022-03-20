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
        size = _size;
        seek = _seek;
        read = _read;
        write = _write;
        close = _close;
    }

    static File& f(SDL_RWops* ctx) {
        return *static_cast<FileOps*>(ctx)->file;
    }

    static Sint64 _size(struct SDL_RWops* context) {
        return f(context).size();
    }

    static Sint64 _seek(struct SDL_RWops* context, Sint64 offset, int whence) {
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

    static size_t _read(struct SDL_RWops * context, void *ptr, size_t size, size_t maxnum) {
        return f(context).read(ptr, size * maxnum) / size;
    }

    static size_t _write(struct SDL_RWops * context, const void *ptr, size_t size, size_t num) {
        return f(context).write(ptr, size * num) / size;
    }

    static int _close(struct SDL_RWops * context) {
        return 0;
    }
};
