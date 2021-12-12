// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>

class Value;

class FSEntity : public Injectable<FSEntity>, public std::enable_shared_from_this<FSEntity> {
public:
    virtual bool isFolder() = 0;
    virtual bool isFile() = 0;
    virtual void init(const Value& resource) = 0;
    static inline String separator;
};
