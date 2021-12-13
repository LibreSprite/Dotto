// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <type_traits>

#include <common/String.hpp>
#include <common/Value.hpp>
#include <Log.hpp>

class PropertySet {
    mutable HashMap<String, std::shared_ptr<Value>> properties;

public:
    template<typename Type>
    bool get(const String& key, Type& out) const {
        auto it = properties.find(tolower(key));
        if (it == properties.end())
            return false;

        if (it->second->has<Type>()) {
            out = it->second->get<Type>();
            return true;
        }

        if constexpr (std::is_same_v<Type, bool>) {
            if (it->second->has<String>()) {
                String str = tolower(it->second->get<String>());
                out = str.size() && (str[0] == 't' || str[0] == 'y');
            } else out = false;
        } else if constexpr (std::is_integral_v<Type>) {
            if (!it->second->has<String>())
                return false;
            out = std::atol(it->second->get<String>().c_str());
        } else if constexpr (std::is_floating_point_v<Type>) {
            if (!it->second->has<String>())
                return false;
            out = std::atof(it->second->get<String>().c_str());
        } else if constexpr (std::is_constructible_v<Type, std::shared_ptr<Value>>) {
            out = Type{it->second};
        } else if constexpr (std::is_assignable_v<Type, std::shared_ptr<Value>>) {
            out = Type{it->second};
        } else if constexpr (std::is_assignable_v<Type, String>) {
            if (!it->second->has<String>())
                return false;
            out = it->second->get<String>();
        } else if constexpr (std::is_constructible_v<Type, String>) {
            if (!it->second->has<String>())
                return false;
            out = Type{it->second->get<String>()};
        } else if constexpr (std::is_assignable_v<Type, Value>) {
            out = *it->second;
        } else if constexpr (std::is_constructible_v<Type, Value>) {
            out = Type{*it->second};
        } else {
            return false;
        }

        *it->second = out;
        return true;
    }

    template<typename Type>
    Type get(const String& key) const {
        Type value{};
        get(key, value);
        return value;
    }

    template<typename Type>
    void set(const String& key, Type& value) {
        properties.insert({tolower(key), std::make_shared<Value>(value)});
    }

    void append(const PropertySet& other) {
        for (auto& entry : other.properties) {
            properties.insert({entry.first, entry.second});
        }
    }

    void prepend(const PropertySet& other) {
        for (auto& entry : other.properties) {
            if (properties.find(entry.first) == properties.end()) {
                properties.insert({entry.first, entry.second});
            }
        }
    }

    void append(std::shared_ptr<PropertySet> other) {
        if (other)
            append(*other);
    }

    void prepend(std::shared_ptr<PropertySet> other) {
        if (other)
            prepend(*other);
    }
};

class Serializable {
    struct PropertySerializer {
        void* property;
        void (*load)(void* property, const PropertySet&);
        void (*store)(void* property, PropertySet&);
    };

    Vector<PropertySerializer> propertySerializers;

protected:
    template<typename _Type>
    class Property {
    public:
        using Type = _Type;
        Type value;
        const String name;

        Property(Serializable* parent, const String& name, const Type& value = Type{}) : name{name} {
            parent->propertySerializers.push_back(PropertySerializer{
                    this,
                    +[](void* data, const PropertySet& set) {
                        auto prop = static_cast<Property<Type>*>(data);
                        bool result = set.get(prop->name, prop->value);
                        logV("Loading ", prop->name, result?" OK":" FAIL");
                    },
                    +[](void* data, PropertySet& set) {
                        auto prop = static_cast<Property<Type>*>(data);
                        set.set(prop->name, prop->value);
                    }
                });
        }

        Type& operator * () {
            return value;
        }

        Type* operator -> () {
            return &value;
        }

        operator Type& () {
            return value;
        }
    };

    void load(const PropertySet& set) {
        for (auto& serializer : propertySerializers) {
            serializer.load(serializer.property, set);
        }
    }

    void store(PropertySet& set) {
        for (auto& serializer : propertySerializers) {
            serializer.store(serializer.property, set);
        }
    }
};
