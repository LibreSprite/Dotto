// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <fs/File.hpp>

class FileSystem : public Injectable<FileSystem> {
protected:
    inject<FSEntity> root{"rootDir"};

public:
    Provides p{this};
    Vector<String> splitPath(const String& path);
    virtual std::shared_ptr<FSEntity> find(const String& path);
    virtual bool boot();
};
