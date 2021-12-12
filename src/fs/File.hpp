// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <log/Log.hpp>
#include <common/Value.hpp>

class FSEntity : public Injectable<FSEntity>, public std::enable_shared_from_this<FSEntity> {
public:
    virtual bool isFolder() = 0;
    virtual bool isFile() = 0;
    virtual void init(const Value& resource) = 0;
    static inline String separator;
};

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
            Log::write(Log::Level::ERROR, String("Invalid resource for Folder: ") + resource.typeName());
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


struct FileOpenSettings {
    bool write = false;
    bool create = true;
};

class File : public FSEntity {
public:
    virtual bool isFolder() {return false;}
    virtual bool isFile() {return true;}

    virtual bool open(const FileOpenSettings& settings = {}) {return true;}
    virtual void close(){}

    virtual bool isOpen() = 0;
    virtual U64 size() = 0;
    virtual bool seek(U64 position) = 0;
    virtual U64 tell() = 0;
    virtual U64 read(void* buffer, U64 size) = 0;
    virtual U64 write(const void* buffer, U64 size) = 0;

    std::string readTextFile() {
        auto len = size();
        std::string str;
        str.resize(len);
        read(&str[0], len);
        return str;
    }

    template<typename Type>
    U64 read(Type& target) {return read(&target, sizeof(target));}

    template<typename Type>
    U64 write(const Type& target) {return write(&target, sizeof(target));}
};
