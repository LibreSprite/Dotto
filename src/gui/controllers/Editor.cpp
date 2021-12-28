// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "log/Log.hpp"
#include <optional>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Document.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>

class Editor : public ui::Controller {
    inject<Document> doc{"new"};
    PubSub<msg::ActivateDocument> pub{this};
    std::optional<Document::Provides> provides;

public:
    void attach() override {
        node()->addEventListener<ui::Focus, ui::FocusChild>(this);
        pub(msg::ActivateDocument{doc});
    }

    void eventHandler(const ui::FocusChild&) {
        pub(msg::ActivateDocument{doc});
    }

    void eventHandler(const ui::Focus&) {
        pub(msg::ActivateDocument{doc});
    }

    void on(msg::ActivateDocument& msg) {
        if (msg.doc.get() == doc.get()) {
            logI("Activating document ", (uintptr_t) doc.get());
            provides.emplace(doc.get(), "activedocument");
        } else {
            logI("Deactivating document ", (uintptr_t) doc.get());
            provides.reset();
        }
    }
};

static ui::Controller::Shared<Editor> reg{"editor"};
