// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <tools/Tool.hpp>

void Tool::save(std::shared_ptr<PropertySet> meta) {
    inject<Config> cfg;
    auto settings = cfg->properties->get<std::shared_ptr<PropertySet>>(_name);
    if (!settings) {
        settings = std::make_shared<PropertySet>();
        cfg->properties->set(_name, settings);
        cfg->dirty();
    }

    auto properties = getMetaProperties();
    auto map = getPropertySet().getMap();
    for (auto& entry : meta->getMap()) {
        auto cps = entry.second->get<std::shared_ptr<PropertySet>>();
        if (auto label = cps->get<String>("label"); !label.empty()) {
            if (auto it = map.find(label); it != map.end()) {
                auto& settingsValue = settings->getMap()[label];
                if (settingsValue != it->second) {
                    settingsValue = it->second;
                    cfg->dirty();
                }
            }
        }
    }
}

void Tool::init(const String& name) {
    _name = name;
    instances.insert({name, shared_from_this()});
    load({
            {"icon", "%appdata/icons/" + name + ".png"},
            {"tool", name},
            {"meta", getMetaProperties()}
        });

    inject<Config> cfg;
    auto settings = cfg->properties->get<std::shared_ptr<PropertySet>>(name);
    if (settings) {
        load(*settings);
    } else {
        settings = std::make_shared<PropertySet>();
        cfg->properties->set(_name, settings);
        cfg->dirty();
    }

    auto properties = getMetaProperties();
    auto& map = getPropertySet().getMap();
    for (auto& entry : properties->getMap()) {
        auto cps = entry.second->get<std::shared_ptr<PropertySet>>();
        if (auto label = cps->get<String>("label"); !label.empty()) {
            if (auto it = map.find(label); it != map.end()) {
                settings->getMap().insert({label, it->second});
                cfg->dirty();
            }
        }
    }
}

void Tool::Preview::drawOutlineSolid(bool clear, Preview& preview, Surface& surface, const Rect& container, F32 scale) {
    auto color = clear ? 0 : preview.overlayColor.toU32();
    Rect dirty;
    bool before = false;
    preview.overlay->apply(
        {
            S32(-container.x / scale),
            S32(-container.y / scale),
            surface.width(),
            surface.height()
        },
        [&](S32 x, S32 y, U8 amount) {
            bool above = preview.overlay->get(x, y - 1) > 127;
            bool current = amount > 127;
            if (above == current && before == current)
                return;
            S32 x0 = x * scale + container.x;
            S32 y0 = y * scale + container.y;
            dirty.expand(x0, y0);
            if (above != current) {
                dirty.expand(x0 + scale + 1, y0);
                surface.setHLine(x0, y0, scale + 1, color);
            }
            if (before != current) {
                before = current;
                dirty.expand(x0, y0 + scale + 1);
                surface.setVLine(x0, y0, scale + 1, color);
            }
        });
    dirty.intersect(surface.rect());
    surface.setDirty(dirty);
}

void Tool::Preview::drawOutlineAnts(bool clear, Preview& preview, Surface& surface, const Rect& container, F32 scale) {
    if (clear) {
        drawOutlineSolid(clear, preview, surface, container, scale);
        return;
    }

    auto color = preview.overlayColor.toU32();
    auto invert = preview.altColor.toU32();
    Rect dirty;
    bool before = false;
    preview.overlay->apply(
        {
            S32(-container.x / scale),
            S32(-container.y / scale),
            surface.width(),
            surface.height()
        },
        [&](S32 x, S32 y, U8 amount) {
            bool above = preview.overlay->get(x, y - 1) > 127;
            bool current = amount > 127;
            if (above == current && before == current)
                return;
            S32 x0 = x * scale + container.x;
            S32 y0 = y * scale + container.y;
            dirty.expand(x0, y0);
            if (above != current) {
                dirty.expand(x0 + scale + 1, y0);
                surface.antsHLine(x0, y0, scale + 1, antAge, color, invert);
            }
            if (before != current) {
                before = current;
                dirty.expand(x0, y0 + scale + 1);
                surface.antsVLine(x0, y0, scale + 1, antAge, color, invert);
            }
        });
    dirty.intersect(surface.rect());
    surface.setDirty(dirty);
}

void Tool::Preview::drawFilledSolid(bool clear, Preview& preview, Surface& surface, const Rect& container, F32 scale) {
    auto color = clear ? 0 : preview.overlayColor.toU32();
    auto invertColor = clear ? 0 : preview.altColor.toU32();
    Rect dirty;
    bool before = false;
    S32 scalei = scale + 0.5f;
    preview.overlay->apply(
        {
            S32(-container.x / scale),
            S32(-container.y / scale),
            surface.width(),
            surface.height()
        },
        [&](S32 x, S32 y, U8 amount) {
            bool above = preview.overlay->get(x, y - 1) > 127;
            bool current = amount > 127;
            S32 x0 = x * scale + 0.5f + container.x;
            S32 y0 = y * scale + 0.5f + container.y;
            if (current) {
                surface.fillRect({x0, y0, U32(scalei) + 1, U32(scalei) + 1}, color);
            }
            if (above == current && before == current)
                return;
            dirty.expand(x0, y0);
            if (above != current) {
                dirty.expand(x0 + scalei + 1, y0);
                surface.setHLine(x0, y0, scalei + 1, invertColor);
            }
            if (before != current) {
                before = current;
                dirty.expand(x0, y0 + scalei + 1);
                surface.setVLine(x0, y0, scalei + 1, invertColor);
            }
        });
    dirty.intersect(surface.rect());
    surface.setDirty(dirty);
}
