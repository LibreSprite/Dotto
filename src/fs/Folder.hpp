// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Value.hpp>
#include <fs/FSEntity.hpp>
#include <log/Log.hpp>

class Folder : public FSEntity {
    struct FSNode {
        Value resource;
        String driver;
        FSNode(const Value& resource, const String& driver) : resource(resource), driver(driver) {}
    };

    HashMap<String, std::shared_ptr<FSNode>> children;
    String path;

public:
    void init(const Value& resource) override {
        if (resource.has<String>()) {
            path = resource.get<String>();
        } else {
            logE(String("Invalid resource for Folder: ") + resource.typeName());
        }
    }

    String getUID() override {return path;}
    bool isFolder() override {return true;}
    bool isFile() override {return false;}

    void forEach(std::function<void(std::shared_ptr<FSEntity>)>);

    std::shared_ptr<FSEntity> getChild(const String& name, const String& missingDriver) {
        std::shared_ptr<FSEntity> driver;
        auto it = children.find(name);
        if (it == children.end()) {
            if (!missingDriver.empty()) {
                driver = inject<FSEntity>{missingDriver};
                if (driver)
                    driver->init(path + separator + name);
            }
        } else {
            driver = inject<FSEntity>{it->second->driver};
            if (driver)
                driver->init(it->second->resource);
        }
        return driver;
    }

    void mount(const String& name, const String& driver, const Value& resource) {
        if (resource.has<String>()) {
            logV("Creating ", driver, " mount point ", name, " to ", resource.get<String>());
        } else {
            logV("Creating ", driver, " mount point ", name, " to ", resource.typeName());
        }
        children.insert({name, std::make_shared<FSNode>(resource, driver)});
    }
};

class RootFolder : public Folder {
public:
    virtual bool boot() = 0;
};
