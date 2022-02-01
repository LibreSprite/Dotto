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
    Property<std::shared_ptr<Document>> doc{this, "doc"};
    PubSub<msg::ActivateDocument, msg::ActivateEditor, msg::PollActiveEditor> pub{this};
    std::optional<Document::Provides> docProvides;
    std::optional<Cell::Provides> cellProvides;
    std::optional<ui::Node::Provides> editorProvides;
    Property<String> filePath{this, "file", "", &Editor::openFile};
    Property<std::shared_ptr<PropertySet>> newFileProperties{this, "newfile", nullptr, &Editor::newFile};
    Property<F32> scale{this, "scale", 1.0f, &Editor::rezoom};
    Property<U32> frame{this, "frame", 0, &Editor::setFrame};
    Property<U32> layer{this, "layer", 0, &Editor::setFrame};
    std::shared_ptr<Cell> lastCell;
    std::shared_ptr<Cell> activeCell;
    U32 activeFrame = -1;
    U32 activeLayer = -1;
    Vector<std::shared_ptr<ui::Node>> cellNodes;

public:
    void setFrame() {
        if (!*doc)
            return;
        auto timeline = (*doc)->currentTimeline();
        if (frame >= timeline->frameCount()) {
            set("frame", timeline->frameCount() - 1);
            return;
        }
        if (layer >= timeline->layerCount()) {
            set("layer", timeline->layerCount() - 1);
            return;
        }

        auto cell = timeline->getCell(frame, layer);
        if (activeCell == cell)
            return;

        activeCell = timeline->getCell(frame, layer);

        if (frame != activeFrame || layer != activeLayer) {
            for (auto cellNode : cellNodes)
                cellNode->remove();
            cellNodes.clear();

            for (U32 i = 0, count = timeline->layerCount(); i < count; ++i) {
                lastCell = timeline->getCell(frame, i);
                if (!lastCell)
                    continue;
                Cell::Provides p{lastCell};
                auto cellNode = ui::Node::fromXML("canvas");
                cellNodes.push_back(cellNode);
                node()->addChild(cellNode);
            }
        }

        cellProvides.emplace(activeCell.get(), "activecell");
        pub(msg::ActivateCell{activeCell});

        if (frame != activeFrame) {
            activeFrame = frame;
            pub(msg::ActivateFrame{doc, frame});
        }

        if (layer != activeLayer) {
            activeLayer = layer;
            pub(msg::ActivateLayer{doc, layer});
        }
    }

    void attach() override {
        node()->addEventListener<ui::Focus, ui::FocusChild>(this);
        pub(msg::ActivateDocument{doc});
    }

    void rezoom() {
        if (!*doc)
            return;
        node()->load({
                {"x", "50%-50%"},
                {"y", "50%-50%"},
                {"width", (*doc)->width() * scale},
                {"height", (*doc)->height() * scale}
            });
    }

    void newFile() {
        if (!*newFileProperties)
            return;
        doc.value = inject<Document>{"new"};
        (*doc)->load(*newFileProperties);
        showFile();
    }

    void openFile() {
        node()->removeAllChildren();
        *doc = inject<Document>{"new"};
        (*doc)->load(!filePath->empty() ? FileSystem::parse(filePath) : Value{});
        showFile();
    }

    void showFile() {
        node()->load({
                {"visible", true},
                {"doc", *doc}
            });
        rezoom();
        setFrame();
        activate();
        node()->focus();
    }

    void activate() {
        editorProvides.emplace(node(), "activeeditor");
        docProvides.emplace(doc->get(), "activedocument");
        cellProvides.emplace(activeCell.get(), "activecell");
        pub(msg::ActivateEditor{node()});
        pub(msg::ActivateDocument{doc});
        pub(msg::ActivateCell{activeCell});
    }

    void eventHandler(const ui::FocusChild&) {activate();}
    void eventHandler(const ui::Focus&) {activate();}

    void on(msg::ActivateDocument& msg) {
        if (msg.doc.get() != doc->get()) {
            docProvides.reset();
            cellProvides.reset();
        }
    }

    void on(msg::ActivateEditor& msg) {
        if (msg.editor != node())
            editorProvides.reset();
    }

    void on(msg::PollActiveEditor& msg) {
        if (editorProvides.has_value())
            msg.editor = node();
    }
};

static ui::Controller::Shared<Editor> reg{"editor"};
