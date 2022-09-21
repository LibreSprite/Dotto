// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Document.hpp>
#include <script/Engine.hpp>
#include <script/ScriptObject.hpp>

class DocumentScriptObject : public script::ScriptObject {
    std::weak_ptr<Document> weak;
    PubSub<> pub{this};

public:
    void postInject() override {
        addFunction("activate", [=]{
            if (auto doc = weak.lock()) {
                pub(msg::ActivateDocument{doc});
                return true;
            }
            return false;
        });

        addProperty("palette", [=]()->script::Value{
            if (auto doc = weak.lock()) {
                return getEngine().toValue(doc->palette());
            } else {
                logI("palette:No document");
            }
            return nullptr;
        });

        addProperty("path", [=]{
            if (auto doc = weak.lock()) {
                return doc->path();
            }
            return String{};
        });
    }

    Value getWrapped() override {
        return weak.lock();
    }

    void setWrapped(const Value& vdoc) override {
        weak = std::shared_ptr<Document>(vdoc);
    }
};

static script::ScriptObject::Shared<DocumentScriptObject> reg{typeid(std::shared_ptr<Document>).name()};
