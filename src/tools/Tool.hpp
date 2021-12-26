// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Color.hpp>
#include <common/inject.hpp>

class Surface;

class Tool : public Injectable<Tool>, public std::enable_shared_from_this<Tool> {
public:
    static inline HashMap<String, std::shared_ptr<Tool>> instances;
    static inline std::weak_ptr<Tool> active;
    static inline Color color;

    static void boot() {
        for (auto& entry : Tool::createAll()) {
            instances.insert({entry.first, entry.second});
        }
    }

    using Path = Vector<Point>;
    std::shared_ptr<Surface> getIcon() {return nullptr;}
    virtual void begin(Surface* surface, const Vector<Point>& points) {}
    virtual void update(Surface* surface, const Vector<Point>& points) {}
    virtual void end(Surface* surface, const Vector<Point>& points) {}
};
