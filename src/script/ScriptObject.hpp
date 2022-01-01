// LibreSprite Scripting Library
// Copyright (c) 2021 LibreSprite contributors
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Value.hpp>
#include <script/Function.hpp>

namespace script {
    class Engine;

    struct ObjectProperty {
        Function getter;
        Function setter;
        String docStr;

        ObjectProperty& doc(const String& str) {
            docStr = str;
            return *this;
        }
    };

    struct DocumentedFunction : public Function {
        struct DocArg {
            String name;
            String docStr;
        };
        String docStr;
        Vector<DocArg> docArgs;
        String docReturnsStr = "Nothing";

        DocumentedFunction(const Function& other) : Function(other) {}

        DocumentedFunction(Function&& other) : Function(std::move(other)) {}

        DocumentedFunction& doc(const String& str) {
            docStr = str;
            return *this;
        }

        DocumentedFunction& docArg(const String& arg, const String& doc) {
            docArgs.emplace_back(DocArg{arg, doc});
            return *this;
        }

        DocumentedFunction& docReturns(const String& doc) {
            docReturnsStr = doc;
            return *this;
        }
    };

    class InternalScriptObject : public Injectable<InternalScriptObject> {
    public:
        InternalScriptObject();
        ~InternalScriptObject();

        static ScriptObject* getScriptObject(void* internal) {
            auto it = liveInstances.find(static_cast<InternalScriptObject*>(internal));
            return it != liveInstances.end() ? (*it)->scriptObject : nullptr;
        }

        virtual void makeGlobal(const String& name);

        virtual ObjectProperty& addProperty(const String& name, const Function& get, const Function& set) {
            auto& prop = properties[name];
            prop.getter = get;
            prop.setter = set;
            return prop;
        }

        virtual DocumentedFunction& addFunction(const String& name, const Function& func) {
            return functions.emplace(name, func).first->second;
        }

        ScriptObject* scriptObject;
        inject<script::Engine> engine;
        HashMap<String, ObjectProperty> properties;
        HashMap<String, DocumentedFunction> functions;
        static inline HashSet<InternalScriptObject*> liveInstances;
    };

    class ScriptObject : public Injectable<ScriptObject>, public std::enable_shared_from_this<ScriptObject> {
        std::shared_ptr<void> held;
        U32 holdCount = 0;

    public:
        InternalScriptObject* getInternalScriptObject() {return internal;};

        ScriptObject() {
            internal->scriptObject = this;
        }

        virtual ::Value getWrapped(){return nullptr;}

        virtual void setWrapped(const ::Value& wrapped){
            if (holdCount)
                held = getWrapped().getShared();
            else
                held.reset();
        }

        void hold(){
            if (!holdCount)
                held = getWrapped().getShared();
            holdCount++;
        }

        void release() {
            if (holdCount) {
                holdCount--;
                if (!holdCount)
                    held.reset();
            }
        }

        script::Engine& getEngine() {return *internal->engine;}

        template <typename Type = Value>
        Type get(const String& name) {
            auto it = internal->properties.find(name);
            if (it == internal->properties.end())
                return Value{};
            it->second.getter();
            return it->second.getter.result;
        }

        void set(const String& name, const Value& value) {
            auto it = internal->properties.find(name);
            if (it != internal->properties.end()) {
                auto& setter = it->second.setter;
                setter.arguments.emplace_back(value);
                setter();
            }
        }

        template <typename RetType = Value, typename ... Args>
        RetType call(const String& name, Args ... args) {
            auto it = internal->functions.find(name);
            if (it == internal->functions.end())
                return Value{};
            auto& func = it->second;
            func.arguments = {args...};
            func();
            return func.result;
        }

    protected:
        void makeGlobal(const String& name) {
            internal->makeGlobal(name);
        }

        ObjectProperty& addProperty(const String& name, const Function& get = []{return Value{};}, const Function &set = [](const Value&){return Value{};}) {
            return internal->addProperty(name, get, set);
        }

        DocumentedFunction& addFunction(const String& name, const Function& func) {
            return internal->addFunction(name, func);
        }

        template<typename Class, typename Ret, typename ... Args>
        DocumentedFunction& addMethod(const String& name, Class* instance, Ret(Class::*method)(Args...)) {
            return addFunction(name, [=](Args ... args){
                return (instance->*method)(std::forward<Args>(args)...);
            });
        }

        template<typename Class, typename ... Args>
        DocumentedFunction& addMethod(const String& name, Class* instance, void(Class::*method)(Args...)) {
            return addFunction(name, [=](Args ... args){
                (instance->*method)(std::forward<Args>(args)...);
                return Value{};
            });
        }

        template<typename Class, typename Ret, typename ... Args>
        DocumentedFunction& addMethod(const String& name, Ret(Class::*method)(Args...)) {
            return addFunction(name, [=](Args ... args){
                return (static_cast<Class*>(this)->*method)(std::forward<Args>(args)...);
            });
        }

        template<typename Class, typename ... Args>
        DocumentedFunction& addMethod(const String& name, void(Class::*method)(Args...)) {
            return addFunction(name, [=](Args ... args){
                (static_cast<Class*>(this)->*method)(std::forward<Args>(args)...);
                return Value{};
            });
        }

        inject<InternalScriptObject> internal;
    };

    class ScriptTarget : public Injectable<ScriptTarget> {
    public:
        Provides provides{this};
        ::Value target;
        ScriptTarget(::Value target) :
            target{target} {}
    };
}
