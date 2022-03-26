// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if !defined(NO_NETWORK)

#include <regex>

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#include <httprequest/httplib.hpp>
#else
#include <httprequest/HTTPRequest.hpp>
#endif

#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>
#include <task/TaskManager.hpp>

class HTTPScriptObject : public script::ScriptObject {
public:
    TaskHandle handle;

    enum class State {
        Ready,
        Busy,
        Error
    } state = State::Ready;

    U32 code = 0;
    Vector<U8> bytes;
    String error;
    String url;
    std::shared_ptr<script::Engine> engine;

    HTTPScriptObject() {
        addMethod("get", this, &HTTPScriptObject::get);
        addProperty("state", [=]{return (int) state;});
        addProperty("code", [=]{return code;});
        addProperty("text", [=]{return String{bytes.begin(), bytes.end()};});
        addProperty("url", [=]{return url;});
        addProperty("error", [=]{return error;});
        makeGlobal("http");
    }

    void get(const String& url){
        engine = getEngine().shared_from_this();

        String event = "get";
        auto& args = script::Function::varArgs();
        if (args.size() >= 2)
            event = args[1].str();

        if (state == State::Busy) {
            logE("Busy, can't request for [", url, "]");
            return;
        }

        code = 0;
        bytes.clear();
        this->url = url;
        state = State::Busy;
        std::weak_ptr<HTTPScriptObject> weak{std::static_pointer_cast<HTTPScriptObject>(shared_from_this())};
        handle = inject<TaskManager>{}->add(
            [=]()->Value{
                try {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
                    static std::regex expr("^(https?://[^/]+)(.*$)");
                    std::cmatch match;
                    std::regex_match(url.c_str(), match, expr);
                    if (match.empty())
                        return "Invalid URL:" + url;
                    httplib::Client req{match[1].str().c_str()};
                    auto resp = req.Get(match[2].str().c_str());
                    if (!resp)
                        return httplib::to_string(resp.error());
                    bytes.insert(bytes.end(), resp->body.begin(), resp->body.end());
                    return int(resp->status);
#else
                    http::Request req{url};
                    auto resp = req.send("GET");
                    bytes = std::move(resp.body);
                    return int(resp.status.code);
#endif
                } catch (const std::exception& ex) {
                    return String{ex.what()};
                }
            },
            [=](Value&& resp){
                auto that = weak.lock();
                if (!that)
                    return;
                if (resp.has<int>()) {
                    that->code = resp.get<int>();
                    that->state = State::Ready;
                } else if (resp.has<String>()) {
                    that->state = State::Error;
                    logI("HTTP Get Error: ", resp.get<String>());
                }
                auto copy = engine;
                engine.reset();
                copy->raiseEvent({event, std::to_string(that->code)});
            });
    }
};

static script::ScriptObject::Shared<HTTPScriptObject> reg("HTTPScriptObject", {"global"});

#endif
