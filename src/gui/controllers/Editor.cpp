// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <optional>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>

class Editor : public ui::Controller {
    std::shared_ptr<Document> doc;
    PubSub<msg::ActivateDocument> pub{this};
    std::optional<Document::Provides> docProvides;
    std::optional<Cell::Provides> cellProvides;
    Property<String> filePath{this, "file", "", &Editor::openFile};
    Property<F32> scale{this, "scale", 1.0f, &Editor::rezoom};
    std::shared_ptr<Cell> lastCell;

public:

    void rezoom() {
        if (!doc)
            return;
        node()->load({
                {"width", doc->width() * scale},
                {"height", doc->height() * scale}
            });
    }

    void openFile() {
        node()->removeAllChildren();
        doc = inject<Document>{"new"};
        doc->load(!filePath->empty() ? FileSystem::parse(filePath) : Value{});

        auto timeline = doc->currentTimeline();
        for (U32 i = 0, count = timeline->layerCount(); i < count; ++i) {
            lastCell = timeline->getCell(0, i);
            Cell::Provides p{lastCell};
            node()->addChild(ui::Node::fromXML("canvas"));
        }

        rezoom();
    }

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
            docProvides.emplace(doc.get(), "activedocument");
            cellProvides.emplace(lastCell, "activecell");
        } else {
            docProvides.reset();
            cellProvides.reset();
        }
    }
};

static ui::Controller::Shared<Editor> reg{"editor"};
