// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <optional>

#include <common/Messages.hpp>
#include <common/PropertySet.hpp>
#include <common/PubSub.hpp>
#include <common/String.hpp>
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
    PubSub<msg::ActivateDocument, msg::ActivateEditor, msg::PollActiveEditor> pub{this};
    std::optional<Document::Provides> docProvides;
    std::optional<Cell::Provides> cellProvides;
    std::optional<Provides> editorProvides;
    Property<String> filePath{this, "file", "", &Editor::openFile};
    Property<std::shared_ptr<PropertySet>> newFileProperties{this, "newfile", nullptr, &Editor::newFile};
    Property<F32> scale{this, "scale", 1.0f, &Editor::rezoom};
    std::shared_ptr<Cell> lastCell;

public:
    void attach() override {
        node()->addEventListener<ui::Focus, ui::FocusChild>(this);
        pub(msg::ActivateDocument{doc});
    }

    void rezoom() {
        if (!doc)
            return;
        node()->load({
                {"x", "50%-50%"},
                {"y", "50%-50%"},
                {"width", doc->width() * scale},
                {"height", doc->height() * scale}
            });
    }

    void newFile() {
        if (!*newFileProperties)
            return;
        doc = inject<Document>{"new"};
        doc->load(*newFileProperties);
        node()->set("visible", true);
        showFile();
    }

    void openFile() {
        node()->removeAllChildren();
        doc = inject<Document>{"new"};
        doc->load(!filePath->empty() ? FileSystem::parse(filePath) : Value{});
        showFile();
    }

    void showFile() {
        auto timeline = doc->currentTimeline();
        for (U32 i = 0, count = timeline->layerCount(); i < count; ++i) {
            lastCell = timeline->getCell(0, i);
            Cell::Provides p{lastCell};
            node()->addChild(ui::Node::fromXML("canvas"));
        }

        rezoom();
        activate();
        node()->set("visible", true);
        node()->focus();
    }

    void activate() {
        pub(msg::ActivateEditor{node()});
        pub(msg::ActivateDocument{doc});
    }

    void eventHandler(const ui::FocusChild&) {activate();}
    void eventHandler(const ui::Focus&) {activate();}

    void on(msg::ActivateDocument& msg) {
        if (msg.doc.get() == doc.get()) {
            docProvides.emplace(doc.get(), "activedocument");
            cellProvides.emplace(lastCell, "activecell");
        } else {
            docProvides.reset();
            cellProvides.reset();
        }
    }

    void on(msg::ActivateEditor& msg) {
        if (msg.editor == node())
            editorProvides.emplace(this);
        else
            editorProvides.reset();
    }

    void on(msg::PollActiveEditor& msg) {
        if (editorProvides.has_value())
            msg.editor = node();
    }
};

static ui::Controller::Shared<Editor> reg{"editor"};
