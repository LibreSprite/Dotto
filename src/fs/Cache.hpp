// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <memory>

#include <common/inject.hpp>
#include <common/Value.hpp>

class Cache : public Injectable<Cache>, public std::enable_shared_from_this<Cache> {
public:
    virtual void flush() = 0;
    virtual Value get(const String& key) = 0;
    virtual void set(const String& key, const Value& resource) = 0;
};
