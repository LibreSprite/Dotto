// LibreSprite Scripting Library
// Copyright (c) 2021 LibreSprite contributors
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
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
        virtual void makeGlobal(const String& name) = 0;

        virtual ObjectProperty& addProperty(const String& name, const Function& get, const Function& set) {
            auto& prop = properties[name];
            prop.getter = get;
            prop.setter = set;
            return prop;
        }

        virtual DocumentedFunction& addFunction(const String& name, const Function& func) {
            return functions.emplace(name, func).first->second;
        }

        std::shared_ptr<script::Engine> engine;
        HashMap<String, ObjectProperty> properties;
        HashMap<String, DocumentedFunction> functions;
    };

    class ScriptObject : public Injectable<ScriptObject> {
    public:
        InternalScriptObject* getInternalScriptObject() {return m_internal;};

        virtual void* getWrapped(){return nullptr;}
        virtual void setWrapped(void*){}

        template <typename Type = Value>
        Type get(const String& name) {
            auto it = m_internal->properties.find(name);
            if (it == m_internal->properties.end())
                return Value{};
            it->second.getter();
            return it->second.getter.result;
        }

        void set(const String& name, const Value& value) {
            auto it = m_internal->properties.find(name);
            if (it != m_internal->properties.end()) {
                auto& setter = it->second.setter;
                setter.arguments.emplace_back(value);
                setter();
            }
        }

        template <typename RetType = Value, typename ... Args>
        RetType call(const String& name, Args ... args) {
            auto it = m_internal->functions.find(name);
            if (it == m_internal->functions.end())
                return Value{};
            auto& func = it->second;
            func.arguments = {args...};
            func();
            return func.result;
        }

        template<typename Type>
        Type* getWrapped(){ return static_cast<Type*>(getWrapped()); }

    protected:
        void makeGlobal(const String& name) {
            m_internal->makeGlobal(name);
        }

        ObjectProperty& addProperty(const String& name, const Function& get = []{return Value{};}, const Function &set = [](const Value&){return Value{};}) {
            return m_internal->addProperty(name, get, set);
        }

        DocumentedFunction& addFunction(const String& name, const Function& func) {
            return m_internal->addFunction(name, func);
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

        inject<InternalScriptObject> m_internal;
    };

}
