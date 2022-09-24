// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <fs/Cache.hpp>
#include <fs/FSEntity.hpp>
#include <fs/File.hpp>
#include <common/Parser.hpp>

using namespace fs;

Value FSEntity::parse(const String& ext, bool silent) {
    auto file = get<File>();
    if (!file) {
        logE("Could not get File to parse");
        return nullptr;
    }

    inject<Cache> cache;
    auto cacheKey = file->getUID();
    Value value = cache->get(cacheKey);
    if (!value.empty())
        return value;

    if (!file->open()) {
        return nullptr;
    }

    if (auto parser = inject<Parser>{silent ? InjectSilent::Yes : InjectSilent::No, ext.empty() ? file->type() : ext}) {
        value = parser->parseFile(file);
        if (!value.empty() && parser->canCache())
            cache->set(cacheKey, value);
    } else if (!silent) {
        logE("No parser for type: ", file->type());
    }

    return value;
}
