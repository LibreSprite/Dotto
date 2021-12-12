// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <log/Log.hpp>
#include <common/Value.hpp>
#include <fs/FSEntity.hpp>

class Folder : public FSEntity {
    struct FSNode {
        Value resource;
        String driver;
        FSNode(const Value& resource, const String& driver) : resource(resource), driver(driver) {}
    };

    HashMap<String, std::shared_ptr<FSNode>> children;
    String path;

public:
    void init(const Value& resource) {
        if (resource.has<String>()) {
            path = resource.get<String>();
        } else {
            logE(String("Invalid resource for Folder: ") + resource.typeName());
        }
    }

    virtual bool isFolder() {return true;}
    virtual bool isFile() {return false;}

    std::shared_ptr<FSEntity> getChild(const String& name, const String& missingDriver) {
        std::shared_ptr<FSEntity> driver;
        auto it = children.find(name);
        if (it == children.end()) {
            driver = inject<FSEntity>{missingDriver};
            if (driver)
                driver->init(path + separator + name);
        } else {
            driver = inject<FSEntity>{it->second->driver};
            if (driver)
                driver->init(it->second->resource);
        }
        return driver;
    }

    void mount(const String& name, const String& driver, const Value& resource) {
        children.insert({name, std::make_shared<FSNode>(resource, driver)});
    }
};
