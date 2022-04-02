// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <functional>
#include <memory>
#include <vector>

// Unlike a shared_ptr, which deletes the ptr when ALL of the owners die,
// the fork_ptr deletes the ptr when ONE of the owners die.
// Use case: Class A and B have a shared connection C. C becomes invalid once either A or B die.
// The cleanup callback is called only for the remaining owners.

template<typename Type>
class fork_ptr {
    struct guarded_t {
        std::shared_ptr<Type> ptr;
        std::vector<std::pair<void*, std::function<void()>>> cleanup;
        guarded_t(const std::shared_ptr<Type>& ptr) : ptr{ptr} {}
    };
    std::shared_ptr<guarded_t> guarded;

public:
    fork_ptr(const std::shared_ptr<Type>& guarded) : guarded{std::make_shared<guarded_t>(guarded)} {}

    fork_ptr(const fork_ptr<Type>& other) : guarded{other.guarded} {}

    fork_ptr(fork_ptr<Type>&& other) {*this = std::move(other);}

    fork_ptr<Type>& operator = (const fork_ptr<Type>& other) {
        reset();
        guarded = other.guarded;
        return *this;
    }

    fork_ptr<Type>& operator = (fork_ptr<Type>&& other) {
        reset();
        guarded = std::move(other.guarded);
        other.guarded.reset();
        if (guarded) {
            for (auto& entry : guarded->cleanup) {
                if (entry.first == &other)
                    entry.first = this;
            }
        }
        return *this;
    }

    template<typename Func>
    void cleanup(Func&& func) {
        guarded->cleanup.emplace_back(this, std::move(func));
    }

    operator bool () {
        return guarded && guarded->ptr;
    }

    Type* operator -> () {
        return guarded->ptr.get();
    }

    Type* get() {
        return guarded ? guarded->ptr.get() : nullptr;
    }

    std::shared_ptr<Type> shared() {
        return guarded ? guarded->ptr : nullptr;
    }

    void reset() {
        if (guarded && guarded->ptr) {
            auto ptr = guarded->ptr;
            guarded->ptr.reset();
            for (auto& entry : guarded->cleanup) {
                if (entry.first != this) {
                    entry.second();
                }
            }
        }
    }

    ~fork_ptr() {
        reset();
    }
};
