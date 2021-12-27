// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include <duk_config.h>
#include <duktape.h>

#include <common/String.hpp>
#include <script/Engine.hpp>

class ScriptException : public std::exception {
    String msg;
public:
    ScriptException(const String& msg) : msg{msg} {}
    const char* what() const throw() {
        return msg.c_str();
    }
};

using namespace script;

class DukEngine : public Engine {
public:
    HashSet<void*> memory;
    duk_hthread* handle = nullptr;

    DukEngine() : handle{duk_create_heap(malloc, realloc, free, this, fatal)} {
        InternalScriptObject::setDefault("DukScriptObject");
        initGlobals();
    }

    ~DukEngine() {
        duk_destroy_heap(handle);
        for (auto ptr : memory)
            ::free(ptr);
    }

    static void fatal(void*, const char* msg) {
        throw ScriptException(msg);
    }

    static void* malloc(void* ctx, duk_size_t size) {
        if (!size)
            return nullptr;
        auto engine = static_cast<DukEngine*>(ctx);
        auto buffer = ::malloc(size);
        engine->memory.insert(buffer);
        return buffer;
    }

    static void* realloc(void* ctx, void* ptr, duk_size_t size) {
        if (!ptr)
            return malloc(ctx, size);
        if (!size) {
            free(ctx, ptr);
            return nullptr;
        }
        auto engine = static_cast<DukEngine*>(ctx);
        engine->memory.erase(ptr);
        auto buffer = ::realloc(ptr, size);
        engine->memory.insert(buffer);
        return buffer;
    }

    static void free(void* ctx, void* ptr) {
        if (ptr) {
            ::free(ptr);
            auto engine = static_cast<DukEngine*>(ctx);
            engine->memory.erase(ptr);
        }
    }

    bool raiseEvent(const Vector<String>& event) override {
        return eval("if (typeof onEvent === \"function\") onEvent(\"" + join(event, "\",\"") + "\");");
    }

    bool eval(const String& code) override {
        InternalScriptObject::setDefault("DukScriptObject");
        bool success = true;
        try {
            if (duk_peval_string(handle, code.c_str()) != 0) {
                log->write(Log::Level::ERROR, "Error: [", duk_safe_to_string(handle, -1), "]");
                success = false;
            }
            duk_pop(handle);
#ifdef _DEBUG
        } catch (const ScriptException& ex) {
#else
        } catch (const std::exception& ex) {
#endif
            log->write(Log::Level::ERROR, ex.what());
            success = false;
        }
        execAfterEval(success);
        return success;
    }
};

static Engine::Shared<DukEngine> registration("duk", {"js"});

class DukScriptObject : public InternalScriptObject {
public:
    static script::Value getValue(duk_context* ctx, int id) {
        auto type = duk_get_type(ctx, id);
        if (type == DUK_TYPE_NUMBER) {
            return duk_get_number(ctx, id);
        } else if (type == DUK_TYPE_STRING) {
            return {duk_get_string(ctx, id)};
        } else if (type == DUK_TYPE_BOOLEAN) {
            return duk_get_boolean(ctx, id);
        } else if (type == DUK_TYPE_NULL) {
            return {};
        } else if (type == DUK_TYPE_OBJECT) {
            duk_size_t size = 0;
            void* buffer = duk_get_buffer_data(ctx, id, &size);
            if (buffer)
                return {buffer, size, false};
            if (duk_get_prop_string(ctx, id, "\0xFFthis")) {
                auto iso = duk_get_pointer(ctx, -1);
                auto so = getScriptObject(iso);
                return so;
            }
        } else if (type == DUK_TYPE_BUFFER) {
            duk_size_t size = 0;
            void* buffer = duk_get_buffer_data(ctx, id, &size);
            if (buffer)
                return {buffer, size, false};
        }
        printf("Type: %d\n", type);
        return {};
    }

    static duk_ret_t callFunc(duk_context* ctx) {
        int argc = duk_get_top(ctx);
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "\0xFFfunc");
        auto& func = *reinterpret_cast<script::Function*>(duk_get_pointer(ctx, -1));
        for (int i = 0; i < argc; ++i) {
            func.arguments.push_back(getValue(ctx, i));
        }
        func();
        return returnValue(ctx, func.result);
    }

    static duk_ret_t returnValue(duk_context* ctx, const script::Value& value) {
        switch (value.type) {
        case script::Value::Type::UNDEFINED:
            return 0;

        case script::Value::Type::INT:
            duk_push_int(ctx, value);
            break;

        case script::Value::Type::DOUBLE:
            duk_push_number(ctx, value);
            break;

        case script::Value::Type::STRING:
            duk_push_string(ctx, value);
            break;

        case script::Value::Type::OBJECT:
            if (auto object = static_cast<ScriptObject*>(value)) {
                static_cast<DukScriptObject*>(object->getInternalScriptObject())->makeLocal();
            } else {
                duk_push_null(ctx);
            }
            break;

        case script::Value::Type::BUFFER: {
            auto& buffer = value.buffer();
            void* out = duk_push_buffer(ctx, buffer.size(), 0);
            memcpy(out, &buffer[0], buffer.size());
            break;
        }

        }

        return 1;
    }

    void pushFunctions() {
        auto handle = static_cast<DukEngine*>(engine.get())->handle;
        for (auto& entry : functions) {
            duk_push_c_function(handle, callFunc, DUK_VARARGS);
            duk_push_pointer(handle, &entry.second);
            duk_put_prop_string(handle, -2, "\0xFFfunc");
            duk_put_prop_string(handle, -2, entry.first.c_str());
        }
    }

    static duk_ret_t getterFunc(duk_context* ctx) {
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "\0xFFfunc");
        auto& prop = *reinterpret_cast<script::ObjectProperty*>(duk_get_pointer(ctx, -1));
        prop.getter();
        returnValue(ctx, prop.getter.result);
        return 1;
    }

    static duk_ret_t setterFunc(duk_context* ctx) {
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "\0xFFfunc");
        auto& prop = *reinterpret_cast<script::ObjectProperty*>(duk_get_pointer(ctx, -1));
        prop.setter.arguments.emplace_back(getValue(ctx, 0));
        prop.setter();
        return 0;
    }

    void pushProperties() {
        auto handle = static_cast<DukEngine*>(engine.get())->handle;
        duk_push_pointer(handle, this);
        duk_put_prop_string(handle, -2, "\0xFFthis");

        for (auto& entry : properties) {
            duk_push_string(handle, entry.first.c_str());
            auto& prop = entry.second;

            int idx = -2;
            duk_uint_t flags = 0;

            flags |= DUK_DEFPROP_HAVE_GETTER;
            duk_push_c_function(handle, getterFunc, 0);
            duk_push_pointer(handle, &prop);
            duk_put_prop_string(handle, -2, "\0xFFfunc");
            --idx;

            flags |= DUK_DEFPROP_HAVE_SETTER;
            duk_push_c_function(handle, setterFunc, 1);
            duk_push_pointer(handle, &prop);
            duk_put_prop_string(handle, -2, "\0xFFfunc");
            --idx;

            duk_def_prop(handle, idx, flags);
        }
    }

    void makeLocal() {
        auto handle = static_cast<DukEngine*>(engine.get())->handle;
        duk_push_object(handle);
        pushFunctions();
        pushProperties();
    }

    void makeGlobal(const String& name) override {
        auto handle = static_cast<DukEngine*>(engine.get())->handle;
        duk_push_global_object(handle);
        duk_push_object(handle);
        pushFunctions();
        pushProperties();
        duk_put_prop_string(handle, -2, name.c_str());
        duk_pop(handle);
    }
};

static InternalScriptObject::Shared<DukScriptObject> dukSO("DukScriptObject");
