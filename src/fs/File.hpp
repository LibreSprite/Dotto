// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <fs/FSEntity.hpp>
#include <log/Log.hpp>

struct FileOpenSettings {
    bool write = false;
    bool create = true;
};

namespace fs {

    class File : public FSEntity {
    public:
        virtual bool isFolder() {return false;}
        virtual bool isFile() {return true;}

        virtual bool open(const FileOpenSettings& settings = {}) {return true;}
        virtual void close(){}

        virtual bool isOpen() = 0;
        virtual U64 size() = 0;
        virtual bool seek(U64 position) = 0;
        virtual U64 tell() = 0;
        virtual U64 read(void* buffer, U64 size) = 0;
        virtual U64 write(const void* buffer, U64 size) = 0;
        virtual String type() = 0;
        virtual String name() = 0;

        String readTextFile() {
            auto len = size();
            String str;
            str.resize(len);
            read(&str[0], len);
            return str;
        }

        template<typename Type>
        U64 read(Type& target) {return read(&target, sizeof(target));}

        template<typename Type>
        U64 write(const Type& target) {return write(&target, sizeof(target));}
    };

}
