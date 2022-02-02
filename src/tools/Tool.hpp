// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Color.hpp>
#include <common/PropertySet.hpp>
#include <common/inject.hpp>

class Surface;

class Tool : public Injectable<Tool>, public Model, public std::enable_shared_from_this<Tool> {
public:
    static inline HashMap<String, std::shared_ptr<Tool>> instances;
    static inline std::weak_ptr<Tool> previous;
    static inline std::weak_ptr<Tool> active;
    static inline Color color;

    static void boot() {
        for (auto& entry : Tool::getAllWithoutFlag("noauto")) {
            entry.second->init(entry.first);
        }
    }

    using Path = Vector<Point2D>;
    Property<bool> enabled{this, "enabled", true};

    virtual void init(const String& name) {
        instances.insert({name, shared_from_this()});
        load({
                {"icon", "%appdata/icons/" + name + ".png"},
                {"tool", name},
                {"meta", getMetaProperties()}
            });
    }

    virtual void onActivate() {
        set("meta", getMetaProperties());
    }
    virtual void begin(Surface* surface, const Vector<Point2D>& points) {}
    virtual void update(Surface* surface, const Vector<Point2D>& points) {}
    virtual void end(Surface* surface, const Vector<Point2D>& points) {}

    virtual std::shared_ptr<PropertySet> getMetaProperties() {return std::make_shared<PropertySet>();}
};
