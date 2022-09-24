// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <fs/File.hpp>

class FileSystem : public Injectable<FileSystem> {
protected:
    inject<fs::FSEntity> root{"rootDir"};

public:
    Provides p{this};
    Vector<String> splitPath(const String& path);
    static String extension(const String& path);
    std::shared_ptr<fs::FSEntity> getRoot() {return root;}
    virtual std::shared_ptr<fs::FSEntity> find(const String& path, const String& missingType = "std");
    virtual bool boot();
    static Value parse(const String& path, const String& ext = "");
    static bool write(const String& path, const Value& data);
};
