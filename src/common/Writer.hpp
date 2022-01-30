// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Value.hpp>
#include <fs/File.hpp>

class Writer : public Injectable<Writer> {
public:
    virtual bool writeFile(const String& path, const Value& data);
    virtual bool writeFile(std::shared_ptr<File> file, const Value& data) = 0;
};

class SimpleImageWriter : public Writer {
    bool writeFile(const String& path, const Value& data) override;
};
