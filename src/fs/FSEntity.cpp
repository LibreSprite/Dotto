// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <fs/FSEntity.hpp>
#include <fs/File.hpp>
#include <common/Parser.hpp>

Value FSEntity::parse() {
    auto file = get<File>();
    if (!file || !file->open())
        return nullptr;
    if (auto parser = inject<Parser>{file->type()})
        return parser->parseFile(file);
    return nullptr;
}
