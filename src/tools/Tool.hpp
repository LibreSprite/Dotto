// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Color.hpp>
#include <common/Config.hpp>
#include <common/Messages.hpp>
#include <common/PropertySet.hpp>
#include <common/PubSub.hpp>
#include <common/inject.hpp>
#include <doc/Selection.hpp>

class Surface;

class Tool : public Injectable<Tool>, public Model, public std::enable_shared_from_this<Tool> {
    PubSub<> pub{this};
public:
    static inline HashMap<String, std::shared_ptr<Tool>> instances;
    static inline std::weak_ptr<Tool> previous;
    static inline std::weak_ptr<Tool> active;
    static inline Color color;

    struct Preview {
        static void drawOutlineSolid(bool clear, Preview& preview, Surface& surface, const Rect& container, F32 scale);
        static void drawOutlineAnts(bool clear, Preview& preview, Surface& surface, const Rect& container, F32 scale);
        static void drawFilledSolid(bool clear, Preview& preview, Surface& surface, const Rect& container, F32 scale);

        bool hideCursor = false;
        std::shared_ptr<Selection> overlay;
        Color overlayColor;
        Color altColor;

        using draw_t = decltype(drawFilledSolid)*;
        draw_t draw = drawFilledSolid;
    };

    static void boot() {
        for (auto& entry : Tool::getAllWithoutFlag("noauto")) {
            entry.second->init(entry.first);
        }
    }

    using Path = Vector<Point3D>;
    Property<bool> enabled{this, "enabled", true};

    virtual void init(const String& name) {
        instances.insert({name, shared_from_this()});
        load({
                {"icon", "%appdata/icons/" + name + ".png"},
                {"tool", name},
                {"meta", getMetaProperties()}
            });

        if (auto properties = inject<Config>{}->properties->get<std::shared_ptr<PropertySet>>(name)) {
            load(*properties);
        }
    }

    virtual Preview* getPreview() {return nullptr;}

    virtual void onActivate() {
        set("meta", getMetaProperties());
    }

    virtual void invalidateMetaMenu() {
        if (auto old = getPropertySet().get<std::shared_ptr<PropertySet>>("meta")) {
            auto current = getMetaProperties();
            set("meta", current);
            pub(msg::InvalidateMetaMenu{old, current});
        }
    }

    virtual void begin(Surface* surface, Path& points, U32 mode) {}
    virtual void update(Surface* surface, Path& points) {}
    virtual void end(Surface* surface, Path& points) {}

    virtual std::shared_ptr<PropertySet> getMetaProperties() {return std::make_shared<PropertySet>();}
};
