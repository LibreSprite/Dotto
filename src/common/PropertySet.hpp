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
    static inline bool debug = false;

public:
    PropertySet() = default;

    PropertySet(const PropertySet& other) = default;

    PropertySet(PropertySet&& other) : properties{std::move(other.properties)} {}

    PropertySet(const std::initializer_list<std::pair<String, Value>>& args) {
        for (auto& entry : args) {
            set(entry.first, entry.second);
        }
    }

    void print() const {
        for (auto& entry : properties) {
            logI("[", entry.first, "] = [", entry.second->toString(), "]");
        }
    }

    std::size_t size() const {
        return properties.size();
    }

    const HashMap<String, std::shared_ptr<Value>>& getMap() const {
        return properties;
    }

    HashMap<String, std::shared_ptr<Value>>& getMap() {
        return properties;
    }

    template<typename Type>
    static bool assignProperty(Value& from, Type& out) {
        if (from.get(out)) {
            return true;
        }

        logV("Could not assign ", from.typeName(), " to ", typeid(Type).name());
        return false;
    }

    template<typename Type>
    bool get(const String& key, Type& out) const {
        auto it = properties.find(key);
        if (it == properties.end())
            it = properties.find(tolower(key));
        if (it == properties.end())
            return false;
        bool success = it->second->get(out);
        if (!success && debug)
            logE("Could not assign ", it->second->typeName(), " to ", typeid(Type).name());
        return success;
    }

    template<typename Type>
    Type get(const String& key) const {
        Type value{};
        get(key, value);
        return value;
    }

    template<typename Type>
    void set(const String& key, Type&& value) {
        auto it = properties.find(key);
        if (it == properties.end()) {
            properties.insert({key, std::make_shared<Value>(std::forward<Type>(value))});
        } else {
            *it->second = value;
        }
    }

    template<typename Type>
    void push(Type&& value) {
        auto key = std::to_string(size());
        auto it = properties.find(key);
        if (it == properties.end()) {
            properties.insert({key, std::make_shared<Value>(std::forward<Type>(value))});
        } else {
            *it->second = value;
        }
    }

    void append(const PropertySet& other) {
        for (auto& entry : other.properties) {
            properties.insert_or_assign(entry.first, entry.second);
        }
    }

    void prepend(const PropertySet& other) {
        for (auto& entry : other.properties) {
            properties.insert({entry.first, entry.second});
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

public:
    virtual ~Serializable() {}

    template<typename _Type, bool Debug = false>
    class Property {
    public:
        using Type = _Type;
        Type value;
        const String name;
        std::function<void()> change;

        template<typename ParentType, typename InitType = Type>
        Property(ParentType* parent, const String& name, InitType&& value = InitType{}, void (ParentType::*change)() = nullptr) : name{name}, value(std::forward<InitType>(value)) {
            if (change) {
                this->change = [=]{(parent->*change)();};
            }
            parent->propertySerializers.push_back(PropertySerializer{
                    this,
                    &this->name,
                    +[](void* data, Value& value) {
                        auto prop = static_cast<Property<Type>*>(data);
                        auto oldValue = prop->value;
                        bool didAssign = value.get(prop->value);
                        bool didChange = !(prop->value == oldValue);
#ifdef _DEBUG
                        if (Debug || PropertySet::debug) {
                            if (!didAssign)
                                logE("Assign to ", prop->name, ": Could not assign ", value.typeName(), " to ", typeid(Type).name());
                            else if (!didChange)
                                logE("Assign to ", prop->name, ": Value did not change (", value.toString(), ")");
                            else if (!prop->change)
                                logE("Assign to ", prop->name, ": No trigger");
                            else
                                logE("Assign to ", prop->name, ": triggered ", value.toString());
                        }
#endif
                        return (didAssign && prop->change && didChange) ? &prop->change : nullptr;
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

        const Type* operator -> () {
            return &value;
        }

        operator const Type& () {
            return value;
        }
    };

public:
    void set(const String& key, const Value& value, bool debug = false) {
        Value copy = value;
        set(key, copy, debug);
    }

    virtual void set(const String& key, Value& value, bool debug = false) {
        PropertySet::debug = debug;
        for (auto& serializer : propertySerializers) {
            if (key == *serializer.key) {
                if (auto trigger = serializer.load(serializer.property, value)) {
                    (*trigger)();
                }
                return;
            }
        }
#ifdef _DEBUG
        if (debug) {
            logI("Could not set ", key, " to [", value.toString(), "]");
        }
#endif
    }

    virtual void load(const PropertySet& set) {
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

    const Vector<String> getPropertyNames() {
        Vector<String> names;
        names.resize(propertySerializers.size());
        for (auto& serializer : propertySerializers)
            names.push_back(*serializer.key);
        return names;
    }
};

class Model : public Serializable {
    PropertySet model;
protected:
    void loadSilent(const PropertySet& set) {
        if (&set != &model)
            model.append(set);
    }

public:
    void load(const PropertySet& set) {
        loadSilent(set);
        Serializable::load(set);
    }

    std::shared_ptr<Value> get(const String& key) {
        auto& map = model.getMap();
        auto it = map.find(key);
        if (it == map.end())
            return nullptr;
        return it->second;
    }

    void set(const String& key, const Value& value, bool debug = false) {
        Value copy = value;
        set(key, copy, debug);
    }

    void set(const String& key, Value& value, bool debug = false) {
        model.set(key, value);
        Serializable::set(key, value, debug);
    }

    const PropertySet& getPropertySet() const {
        return model;
    }
};

class InjectableModel : public Model,
                        public Injectable<InjectableModel>,
                        public std::enable_shared_from_this<InjectableModel> {
public:
};
