// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/fork_ptr.hpp>
#include <common/types.hpp>

class Rect;

class Texture {
public:
    virtual ~Texture() = default;
    virtual void setDirty(const Rect& region) = 0;
};

class TextureInfo {
    struct Info {
        const void* key = nullptr;
        fork_ptr<Texture> value;
    };
    Vector<Info> infoset;

public:
    Info* find(const void* key) {
        for (auto& entry : infoset) {
            if (entry.key == key)
                return &entry;
        }
        return nullptr;
    }

    void set(void* key, fork_ptr<Texture> value) {
        value.cleanup([=]{erase(key);});
        if (auto it = find(key)) {
            it->value = std::move(value);
        } else {
            infoset.push_back({key, std::move(value)});
        }
    }

    void erase(void* key) {
        for (U32 i = 0, size=infoset.size(); i < size;) {
            if (infoset[i].key == key) {
                if (i != size - 1) {
                    infoset[i] = std::move(infoset.back());
                }
                infoset.pop_back();
                --size;
            } else {
                ++i;
            }
        }
    }

    template<typename Value = Texture>
    std::shared_ptr<Value> get(const void* key) {
        if (auto it = find(key)) {
            return std::dynamic_pointer_cast<Value>(it->value.shared());
        } else {
            return nullptr;
        }
    }

    virtual ~TextureInfo() = default;

    virtual void setDirty(const Rect& region) {
        for (auto& entry : infoset) {
            if (entry.value) {
                entry.value->setDirty(region);
            }
        }
    }
};
