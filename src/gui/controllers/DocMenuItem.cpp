// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "common/Messages.hpp"
#include "common/PubSub.hpp"
#include "doc/Document.hpp"
#include "fs/FileSystem.hpp"
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class DocMenuItem : public ui::Controller {
public:
    Property<Document*> _doc{this, "doc", nullptr, &DocMenuItem::setDoc};
    Property<S32> maxSize{this, "maxsize", 32};
    PubSub<msg::ActivateDocument,
           msg::RenameDocument,
           msg::CloseDocument> pub{this};
    std::weak_ptr<Document> weakDoc;

    void setDoc() {
        String path;
        if (_doc) {
            inject<FileSystem> fs;
            weakDoc = (*_doc)->shared_from_this();
            path = (*_doc)->path();
            path = split(fs->splitPath(path).back(), ".").front();
            if (path.size() > maxSize)
                path.erase(0, path.size() - maxSize);
        } else {
            weakDoc.reset();
        }
        node()->set("label", path);
    }

    void attach() override {
        node()->addEventListener<ui::Click>(this);
    }

    void eventHandler(const ui::Click&) {
        if (auto doc = weakDoc.lock()) {
            pub(msg::RequestActivateDocument(doc));
        }
    }

    void on(msg::RenameDocument& msg) {
        if (_doc == msg.doc.get()) {
            setDoc();
        }
    }

    void on(msg::ActivateDocument& msg) {
        if (!_doc) {
            set("doc", msg.doc.get());
        }
        if (msg.doc.get() == _doc) {
            node()->set("state", "active");
        } else {
            node()->set("state", "enabled");
        }
    }

    void on(msg::CloseDocument& msg) {
        if (msg.doc == _doc)
            node()->remove();
    }
};

static ui::Controller::Shared<DocMenuItem> reg{"docmenuitem"};
