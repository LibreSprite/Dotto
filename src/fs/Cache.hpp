// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <memory>

#include <common/inject.hpp>
#include <common/Value.hpp>

class Cache : public Injectable<Cache>, public std::enable_shared_from_this<Cache> {
    U32 lockCount = 0;
public:
    virtual void flush() = 0;
    virtual Value get(const String& key) = 0;
    virtual void set(const String& key, const Value& resource) = 0;

    class Lock {
        Cache& cache;
    public:
        Lock(Cache& cache) : cache{cache} {
            cache.lockCount++;
        }
        ~Lock(){
            cache.lockCount--;
        }
    };

    Lock lock() {
        return *this;
    }

    bool locked() {
        return lockCount > 0;
    }
};
