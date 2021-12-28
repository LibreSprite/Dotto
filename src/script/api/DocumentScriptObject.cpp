// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Document.hpp>
#include <script/ScriptObject.hpp>

class DocumentScriptObject : public script::ScriptObject {
    std::weak_ptr<Document> weak;
    PubSub<> pub{this};

public:
    Value getWrapped() override {
        return weak.lock();
    }

    void setWrapped(const Value& vdoc) override {
        std::shared_ptr<Document> doc = vdoc;
        weak = doc;
        auto weak = this->weak;
        addFunction("activate", [=]{
            if (auto doc = weak.lock()) {
                pub(msg::ActivateDocument{doc});
                return true;
            }
            return false;
        });
    }
};

static script::ScriptObject::Shared<DocumentScriptObject> reg{typeid(std::shared_ptr<Document>).name()};
