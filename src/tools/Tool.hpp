// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>

class Surface;

class Tool : public Injectable<Tool>, public std::enable_shared_from_this<Tool> {
public:
    static inline Vector<inject<Tool>> instances;
    static inline std::weak_ptr<Tool> active;
    static void boot() {
        instances = Tool::createAll();
        active = Tool::instances[0].shared();
    }

    using Path = Vector<Point>;
    std::shared_ptr<Surface> getIcon() {return nullptr;}
    virtual void begin(Surface* surface, const Vector<Point>& points) {}
    virtual void update(Surface* surface, const Vector<Point>& points) {}
    virtual void end(Surface* surface, const Vector<Point>& points) {}
};
