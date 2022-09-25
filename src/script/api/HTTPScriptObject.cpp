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
    enum class Method {
        Get,
        Post,
    };

    HTTPScriptObject() {
        addFunction("get", [=]{request(Method::Get); return true;});
        addFunction("post", [=]{request(Method::Post); return true;});
        makeGlobal("http");
    }

    void request(Method method){
        struct Request {
            int status = 400;
            String url;
            String postBody;
            String contentType = "application/json";
            Method method;
            String result;
            TaskHandle handle;
            std::shared_ptr<script::EngineObjRef> callback;
            std::shared_ptr<script::Engine> engine;
        };
        auto request = std::make_shared<Request>();

        request->method = method;
        request->engine = getEngine().shared_from_this();

        String key;
        auto& args = script::Function::varArgs();
        for (std::size_t i = 0, max = args.size(); i + 1 < max; i += 2) {
            key = args[i].str();
            auto& val = args[i + 1];
            if (i == 0) {
                request->url = key;
                request->callback = val;
            } else if (key == "body") {
                request->postBody = val.str();
            } else if (key == "contentType") {
                request->contentType = val.str();
            }
        }

        request->handle = inject<TaskManager>{}->add(
            [=]()->Value{
                try {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
                    static std::regex expr("^(https?://[^/]+)(.*$)");
                    std::cmatch match;
                    std::regex_match(request->url.c_str(), match, expr);

                    if (match.empty()) {
                        logI(request->result = "Invalid URL:" + request->url);
                        return request;
                    }

                    httplib::Client req{match[1].str().c_str()};
                    auto resp = request->method == Method::Get ? req.Get(match[2].str().c_str()) :
                        req.Post(match[2].str().c_str(), request->postBody, request->contentType.c_str());

                    if (resp) {
                        request->result = std::move(resp->body);
                        request->status = resp->status;
                    } else {
                        request->result = httplib::to_string(resp.error());
                    }
#else
                    http::Request req{request->url};
                    auto resp = req.send("GET");
                    request->result = String{
                        reinterpret_cast<char*>(resp.body.data()),
                        reinterpret_cast<char*>(resp.body.data() + resp.body.size())
                    };
#endif
                } catch (const std::exception& ex) {
                    request->result = ex.what();
                }
                return request;
            },
            [=](Value&& vreq){
                logI("End req");
                if (auto req = vreq.get<std::shared_ptr<Request>>(); req && req->callback) {
                    logI("Got req ", req->status);
                    req->callback->call({req->status, std::move(req->result)});
                }
            });
    }
};

static script::ScriptObject::Shared<HTTPScriptObject> reg("HTTPScriptObject", {"global"});

#endif
