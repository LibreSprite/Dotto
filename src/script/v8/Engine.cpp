// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if SCRIPT_ENGINE_V8

#include <cstring>
#include <iostream>

#include <v8.h>
#include <libplatform/libplatform.h>

#include <common/String.hpp>
#include <fs/FileSystem.hpp>
#include <script/Engine.hpp>

using namespace script;

template<typename Inner>
v8::Local<Inner> ToLocal(v8::Local<Inner> thing) {
    return thing;
}

template<typename Inner>
v8::Local<Inner> ToLocal(v8::MaybeLocal<Inner> thing) {
    if (thing.IsEmpty()) return {};
    return thing.ToLocalChecked();
}

template<typename T>
void Check(const T&){}

class V8Engine : public Engine {
public:
    v8::Global<v8::Context> m_context;
    v8::Isolate* m_isolate = nullptr;

    V8Engine() : Engine{"V8ScriptObject"} {
        initV8();
    }

    v8::Local<v8::Context> context() { return m_context.Get(m_isolate); }

    void initV8() {
        static std::unique_ptr<v8::Platform> m_platform;
        if (!m_platform) {
            // Conflicting documentation. Not sure if this is actually needed.
            // v8::V8::InitializeICUDefaultLocation(base::get_app_path().c_str());
            // v8::V8::InitializeExternalStartupData(base::get_app_path().c_str());
            v8::V8::InitializeICU();

            m_platform = v8::platform::NewDefaultPlatform();
            v8::V8::InitializePlatform(m_platform.get());
            v8::V8::Initialize();
        }

        v8::Isolate::CreateParams params;
        params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        m_isolate = v8::Isolate::New(params);

        v8::Isolate::Scope isolatescope(m_isolate);
        v8::HandleScope handle_scope(m_isolate);
        m_context = v8::Global<v8::Context>(m_isolate, v8::Context::New(m_isolate));
    }

    bool raiseEvent(const Vector<String>& event) override {
        PushDefault engine{this};
        InternalScriptObject::PushDefault iso{internalScriptObjectName};

        auto lock = shared_from_this();
        bool success = true;
        try {
            v8::Isolate::Scope isolatescope(m_isolate);
            // Create a stack-allocated handle scope.
            v8::HandleScope handle_scope(m_isolate);

            // Enter the context for compiling and running the hello world script.
            v8::Context::Scope context_scope(context());

            v8::TryCatch trycatch(m_isolate);

            auto global = context()->Global();

            auto onEvent = ToLocal(global->Get(context(), ToLocal(v8::String::NewFromUtf8(m_isolate, "onEvent"))));
            if (!onEvent.IsEmpty() && onEvent->IsFunction()) {
                Vector<v8::Local<v8::Value>> argv;
                argv.reserve(event.size());
                for (auto& str : event) {
                    argv.emplace_back(ToLocal(v8::String::NewFromUtf8(m_isolate, str.c_str())));
                }
                Check(onEvent.As<v8::Function>()->Call(context(), global, event.size(), argv.data()));
            }

            if (trycatch.HasCaught()) {
                v8::Local<v8::Value> exception = trycatch.Exception();
                auto trace = trycatch.StackTrace(context());

                v8::String::Utf8Value utf8(m_isolate, exception);

                if (!trace.IsEmpty()){
                    v8::String::Utf8Value utf8Trace(m_isolate, ToLocal(trace));
                    log->write(Log::Level::ERROR, *utf8Trace);
                } else {
                    log->write(Log::Level::ERROR, *utf8);
                }
                success = false;
            }

        } catch (const std::exception& ex) {
            log->write(Log::Level::ERROR, ex.what());
            success = false;
        }
        execAfterEval(success);
        return success;
    }

    bool eval(const String& code) override {
        PushDefault engine{this};
        InternalScriptObject::PushDefault iso{internalScriptObjectName};

        auto lock = shared_from_this();
        bool success = true;
        try {
            v8::Isolate::Scope isolatescope(m_isolate);
            // Create a stack-allocated handle scope.
            v8::HandleScope handle_scope(m_isolate);

            // Enter the context for compiling and running the hello world script.
            v8::Context::Scope context_scope(context());

            v8::TryCatch trycatch(m_isolate);

            initGlobals();

            // Create a string containing the JavaScript source code.
            v8::Local<v8::String> source = ToLocal(v8::String::NewFromUtf8(m_isolate, code.c_str()));

            // Compile the source code.
            v8::MaybeLocal<v8::Script> script = v8::Script::Compile(context(), source);
            // Run the script to get the result.
            v8::MaybeLocal<v8::Value> result;
            if (!script.IsEmpty()) {
                result = ToLocal(script)->Run(context());
            }

            if (result.IsEmpty()) {
                if (trycatch.HasCaught()) {
                    v8::Local<v8::Value> exception = trycatch.Exception();
                    auto trace = trycatch.StackTrace(context());

                    v8::String::Utf8Value utf8(m_isolate, exception);

                    if (!trace.IsEmpty()){
                        v8::String::Utf8Value utf8Trace(m_isolate, ToLocal(trace));
                        log->write(Log::Level::ERROR, *utf8Trace);
                    } else {
                        log->write(Log::Level::ERROR, *utf8);
                    }

                    success = false;
                }
            }
        } catch (const std::exception& ex) {
            log->write(Log::Level::ERROR, ex.what());
            success = false;
        }
        execAfterEval(success);
        return success;
    }
};

static Engine::Shared<V8Engine> registration("js", {"js"});

class V8ScriptObject : public InternalScriptObject {
public:

    static script::Value getValue(v8::Isolate *isolate, v8::Local<v8::Value> local) {
        if (local.IsEmpty() || local->IsNullOrUndefined())
            return {};

        if (local->IsString()) {
            v8::String::Utf8Value utf8(isolate, local);
            return {*utf8};
        }

        if (local->IsNumber())
            return local.As<v8::Number>()->Value();

        if (local->IsUint32())
            return local.As<v8::Uint32>()->Value();

        if (local->IsInt32())
            return local.As<v8::Int32>()->Value();

        if (local->IsBoolean())
            return local.As<v8::Boolean>()->Value();

        if (local->IsUint8Array()){
            auto array = local.As<v8::Uint8Array>();
#if V8_MAJOR_VERSION > 7
            auto store = array->Buffer()->GetBackingStore();
#else
            auto storeObj = array->Buffer()->GetContents();
            auto store = &storeObj;
#endif
            return {
                store->Data(),
                store->ByteLength(),
                false
            };
        }

        if (local->IsObject()) {
            auto obj = local.As<v8::Object>();
            auto prop = ToLocal(obj->Get(isolate->GetCurrentContext(),
                                         ToLocal(v8::String::NewFromUtf8(isolate, "_this_"))));
            if (prop->IsExternal()) {
                return getScriptObject(prop.As<v8::External>()->Value());
            }
        }

        v8::String::Utf8Value utf8(isolate, local->TypeOf(isolate));
        printf("Unknown type: [%s]\n", *utf8);
        return {};
    }

    static v8::Local<v8::Value> returnValue(v8::Isolate* isolate, const script::Value& value) {
        switch (value.type) {
        case script::Value::Type::UNDEFINED:
            return v8::Local<v8::Value>();

        case script::Value::Type::INT:
            return v8::Int32::New(isolate, value);

        case script::Value::Type::DOUBLE:
            return v8::Number::New(isolate, value);

        case script::Value::Type::STRING:
            return ToLocal(v8::String::NewFromUtf8(isolate, value));

        case script::Value::Type::OBJECT:
            if (auto object = static_cast<ScriptObject*>(value)) {
                return static_cast<V8ScriptObject*>(object->getInternalScriptObject())->makeLocal();
            }
            return {};

        case script::Value::Type::BUFFER: {
            auto& buffer = value.buffer();
            if (buffer.canSteal()) {
#if V8_MAJOR_VERSION > 7
                auto store = v8::ArrayBuffer::NewBackingStore(
                    buffer.steal(),
                    buffer.size(),
                    +[](void* data, size_t length, void* deleter_data){
                        delete[] static_cast<uint8_t*>(data);
                    },
                    nullptr
                    );
                auto arrayBuffer = v8::ArrayBuffer::New(isolate, std::move(store));
                return v8::Uint8Array::New(arrayBuffer, 0, buffer.size());
#else
                auto arrayBuffer = v8::ArrayBuffer::New(isolate, buffer.steal(), buffer.size());
#endif
                return v8::Uint8Array::New(arrayBuffer, 0, buffer.size());

            } else {
                auto arrayBuffer = v8::ArrayBuffer::New(isolate, buffer.size());
#if V8_MAJOR_VERSION > 7
                std::memcpy(arrayBuffer->GetBackingStore()->Data(), buffer.data(), buffer.size());
#else
                std::memcpy(arrayBuffer->GetContents().Data(), buffer.data(), buffer.size());
#endif
                return v8::Uint8Array::New(arrayBuffer, 0, buffer.size());
            }
        }

        default:
            printf("Unknown return type: %d\n", int(value.type));
            break;
        }
        return {};
    }

    static void callFunc(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = args.GetIsolate();
        v8::HandleScope handle_scope(isolate);
        auto data = args.Data().As<v8::External>();
        auto& func = *reinterpret_cast<script::Function*>(data->Value());

        for (int i = 0; i < args.Length(); i++) {
            func.arguments.push_back(getValue(isolate, args[i]));
        }

        func();

        args.GetReturnValue().Set(returnValue(isolate, func.result));
    }

    void pushFunctions(v8::Local<v8::Object>& object) {
        auto isolate = engine.get<V8Engine>()->m_isolate;
        auto context = engine.get<V8Engine>()->context();
        for (auto& entry : functions) {
            auto tpl = v8::FunctionTemplate::New(isolate, callFunc, v8::External::New(isolate, &entry.second));
            auto func = tpl->GetFunction(context).ToLocalChecked();
            Check(object->Set(context,
                              ToLocal(v8::String::NewFromUtf8(isolate, entry.first.c_str())),
                              func));
        }
    }

    void pushProperties(v8::Local<v8::Object>& object) {
        auto& isolate = engine.get<V8Engine>()->m_isolate;
        auto context = engine.get<V8Engine>()->context();

        Check(object->Set(context,
                          ToLocal(v8::String::NewFromUtf8(isolate, "_this_")),
                          v8::External::New(isolate, this)));

        for (auto& entry : properties) {
            auto getterTpl = v8::FunctionTemplate::New(isolate, callFunc, v8::External::New(isolate, &entry.second.getter));
            auto getter = getterTpl->GetFunction(context).ToLocalChecked();

            auto setterTpl = v8::FunctionTemplate::New(isolate, callFunc, v8::External::New(isolate, &entry.second.setter));
            auto setter = setterTpl->GetFunction(context).ToLocalChecked();

            v8::PropertyDescriptor descriptor(getter, setter);
            Check(object->DefineProperty(context,
                                         ToLocal(v8::String::NewFromUtf8(isolate, entry.first.c_str())),
                                         descriptor));
        }
    }

    v8::Global<v8::Object> local;

    v8::Local<v8::Object> makeLocal() {
        auto isolate = engine.get<V8Engine>()->m_isolate;
        if (!local.IsEmpty())
            return local.Get(isolate);
        auto obj = v8::Object::New(isolate);
        pushFunctions(obj);
        pushProperties(obj);
        local.Reset(isolate, obj);
        return obj;
    }

    void makeGlobal(const String& name) override {
        InternalScriptObject::makeGlobal(name);
        auto& isolate = engine.get<V8Engine>()->m_isolate;
        auto context = engine.get<V8Engine>()->context();
        Check(context->Global()->Set(context,
                                     ToLocal(v8::String::NewFromUtf8(isolate, name.c_str())),
                                     makeLocal()));
    }
};

static InternalScriptObject::Shared<V8ScriptObject> v8SO("V8ScriptObject");

#endif
