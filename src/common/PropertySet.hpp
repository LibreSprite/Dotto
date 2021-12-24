// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <type_traits>

#include <common/inject.hpp>
#include <common/String.hpp>
#include <common/Value.hpp>
#include <log/Log.hpp>

class PropertySet {
    friend class Serializable;
    mutable HashMap<String, std::shared_ptr<Value>> properties;

public:
    PropertySet() = default;

    PropertySet(const PropertySet& other) = default;

    PropertySet(PropertySet&& other) : properties{std::move(other.properties)} {}

    PropertySet(const std::initializer_list<std::pair<String, Value>>& args) {
        for (auto& entry : args) {
            set(entry.first, entry.second);
        }
    }

    const HashMap<String, std::shared_ptr<Value>>& getMap() const {
        return properties;
    }

    template<typename Type>
    static bool assignProperty(Value& from, Type& out) {
        if (from.has<Type>()) {
            out = from.get<Type>();
            return true;
        }

        if constexpr (std::is_same_v<Type, bool>) {
            if (from.has<String>()) {
                String str = tolower(from.get<String>());
                out = str.size() && (str[0] == 't' || str[0] == 'y');
            } else out = false;
        } else if constexpr (std::is_integral_v<Type>) {
            if (!from.has<String>())
                return false;
            out = std::atol(from.get<String>().c_str());
        } else if constexpr (std::is_floating_point_v<Type>) {
            if (!from.has<String>())
                return false;
            out = std::atof(from.get<String>().c_str());
        } else if constexpr (std::is_constructible_v<Type, std::shared_ptr<Value>>) {
            out = Type{from};
        } else if constexpr (std::is_assignable_v<Type, std::shared_ptr<Value>>) {
            out = Type{from};
        } else if constexpr (std::is_assignable_v<Type, String>) {
            if (!from.has<String>())
                return false;
            out = from.get<String>();
        } else if constexpr (std::is_constructible_v<Type, String>) {
            if (!from.has<String>())
                return false;
            out = Type{from.get<String>()};
        } else if constexpr (std::is_assignable_v<Type, Value>) {
            out = from;
        } else if constexpr (std::is_constructible_v<Type, Value>) {
            out = Type{from};
        } else {
            return false;
        }

        from = out;
        return true;
    }

    template<typename Type>
    bool get(const String& key, Type& out) const {
        auto it = properties.find(tolower(key));
        if (it == properties.end())
            return false;
        return assignProperty(*it->second, out);
    }

    template<typename Type>
    Type get(const String& key) const {
        Type value{};
        get(key, value);
        return value;
    }

    template<typename Type>
    void set(const String& key, Type&& value) {
        properties.insert({tolower(key), std::make_shared<Value>(std::forward<Type>(value))});
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
        const String* key;
        std::function<void()>* (*load)(void* property, Value&);
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
        std::function<void()> change;

        template<typename ParentType>
        Property(ParentType* parent, const String& name, const Type& value = Type{}, void (ParentType::*change)() = nullptr) : name{name}, value{value} {
            if (change) {
                this->change = [=]{(parent->*change)();};
            }
            parent->propertySerializers.push_back(PropertySerializer{
                    this,
                    &this->name,
                    +[](void* data, Value& value) {
                        auto prop = static_cast<Property<Type>*>(data);
                        auto oldValue = prop->value;
                        bool didAssign = PropertySet::assignProperty(value, prop->value);
                        return (didAssign && prop->change && !(prop->value == oldValue)) ? &prop->change : nullptr;
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

    void set(const String& key, Value& value) {
        for (auto& serializer : propertySerializers) {
            if (key == *serializer.key) {
                if (auto trigger = serializer.load(serializer.property, value))
                    (*trigger)();
                return;
            }
        }
    }

    void load(const PropertySet& set) {
        Vector<std::function<void()>*> queue;
        for (auto& serializer : propertySerializers) {
            auto it = set.properties.find(*serializer.key);
            if (it != set.properties.end()) {
                if (auto trigger = serializer.load(serializer.property, *it->second)) {
                    queue.push_back(trigger);
                }
            }
        }

        for (auto trigger : queue) {
            (*trigger)();
        }
    }

    void store(PropertySet& set) {
        for (auto& serializer : propertySerializers) {
            serializer.store(serializer.property, set);
        }
    }

};

class Model : public Serializable {
    PropertySet model;
protected:
    void loadSilent(const PropertySet& set) {
        model.append(set);
    }

public:
    void load(const PropertySet& set) {
        loadSilent(set);
        Serializable::load(set);
    }

    void set(const String& key, const Value& value) {
        Value copy = value;
        set(key, copy);
    }

    void set(const String& key, Value& value) {
        model.set(key, value);
        Serializable::set(key, value);
    }

    const PropertySet& getPropertySet() {
        return model;
    }
};

class InjectableModel : public Model,
                        public Injectable<InjectableModel>,
                        public std::enable_shared_from_this<InjectableModel> {
public:
};
