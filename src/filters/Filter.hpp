// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Color.hpp>
#include <common/PropertySet.hpp>
#include <common/inject.hpp>

class Surface;

class Filter : public Injectable<Filter>, public Model, public std::enable_shared_from_this<Filter> {
public:
    static inline HashMap<String, std::shared_ptr<Filter>> instances;
    static inline std::weak_ptr<Filter> active;

    static void boot() {
        for (auto& entry : Filter::getAllWithoutFlag("noauto")) {
            entry.second->init(entry.first);
        }
    }

    using Path = Vector<Point2D>;
    Property<bool> enabled{this, "enabled", true};

    virtual void init(const String& name) {
        instances.insert({name, shared_from_this()});
        load({
                {"icon", "%skin/" + name + ".png"},
                {"filter", name}
            });
    }

    virtual std::shared_ptr<PropertySet> getMetaProperties() {return std::make_shared<PropertySet>();}

    virtual void run(std::shared_ptr<Surface> surface) = 0;
};
