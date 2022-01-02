// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "log/Log.hpp"
#include <common/PubSub.hpp>
#include <common/Messages.hpp>
#include <fs/Cache.hpp>

class CacheImpl : public Cache {
public:
    HashMap<String, Value> index;
    PubSub<msg::Shutdown> pub{this};

    void flush() override {
        if (locked())
            return;

        for (auto it = index.begin(); it != index.end();) {
            if (pub(msg::Flush{it->second}).isHeld()) {
                ++it;
            } else {
                logV("Cache flush ", it->first);
                it = index.erase(it);
            }
        }
    }

    Value get(const String& key) override {
        auto it = index.find(key);
        return it == index.end() ? Value{} : it->second;
    }

    void set(const String& key, const Value& resource) override {
        flush();
        index[key] = resource;
    }

    void on(msg::Shutdown&) {
        index.clear();
    }
};

static Cache::Shared<CacheImpl> cache{"new"};
