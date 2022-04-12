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
    Property<bool> allLayers{this, "all-layers", false};
    Property<bool> allFrames{this, "all-frames", false};
    std::shared_ptr<PropertySet> undoData;

    virtual bool forceAllLayers() {return false;}
    virtual bool forceAllFrames() {return false;}
    virtual String category() = 0;

    virtual void init(const String& name) {
        instances.insert({name, shared_from_this()});
        load({
                {"icon", "%skin/" + name + ".png"},
                {"filter", name},
                {"category", category()}
            });
    }

    virtual void undo() {}

    virtual std::shared_ptr<PropertySet> getMetaProperties() {
        auto meta = std::make_shared<PropertySet>();
        if (!forceAllFrames()) {
            meta->push(std::make_shared<PropertySet>(PropertySet{
                        {"widget", "checkbox"},
                        {"label", allFrames.name},
                        {"value", allFrames.value}
                    }));
        }
        if (!forceAllLayers()) {
            meta->push(std::make_shared<PropertySet>(PropertySet{
                        {"widget", "checkbox"},
                        {"label", allLayers.name},
                        {"value", allLayers.value}
                    }));
        }
        return meta;
    }

    virtual void beforeRun() {}
    virtual void run(std::shared_ptr<Surface> surface) = 0;
    virtual void afterRun() {}
};
