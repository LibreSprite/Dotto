// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <optional>

#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PropertySet.hpp>
#include <common/PubSub.hpp>
#include <common/String.hpp>
#include <common/System.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class Editor : public ui::Controller {
    Property<std::shared_ptr<Document>> doc{this, "doc"};
    PubSub<msg::ResizeDocument,
           msg::ActivateDocument,
           msg::ActivateEditor,
           msg::ActivateTool,
           msg::PollActiveEditor> pub{this};
    std::optional<Document::Provides> docProvides;
    std::optional<ui::Node::Provides> editorProvides;
    Property<String> filePath{this, "file", "", &Editor::openFile};
    Property<std::shared_ptr<PropertySet>> newFileProperties{this, "newfile", nullptr, &Editor::newFile};
    Property<F32> scale{this, "scale", 1.0f, &Editor::rezoom};
    Property<U32> frame{this, "frame", 0, &Editor::setFrame};
    Property<U32> layer{this, "layer", 0, &Editor::setFrame};
    std::shared_ptr<ui::Node> canvas;
    std::shared_ptr<ui::Node> container;
    std::shared_ptr<Cell> lastCell;
    std::shared_ptr<Cell> activeCell;
    U32 activeFrame = -1;
    U32 activeLayer = -1;
    inject<System> system;

    U32 prevButtons = ~U32{};
    std::shared_ptr<Tool> activeTool;
    Tool::Path points;

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

        auto cell = timeline->activate(frame, layer);
        if (activeCell == cell)
            return;

        activeCell = cell;

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
        node()->addEventListener<ui::Focus,
                                 ui::FocusChild,
                                 ui::MouseMove,
                                 ui::MouseDown,
                                 ui::MouseUp,
                                 ui::MouseWheel,
                                 ui::MouseLeave>(this);
        pub(msg::ActivateDocument{doc});
        canvas = node()->findChildById("canvas");
        container = node()->findChildById("container");
    }

    void eventHandler(const ui::MouseDown& event) {
        paint(event.globalX - canvas->globalRect.x, event.globalY - canvas->globalRect.y, event.buttons);
    }

    void eventHandler(const ui::MouseUp& event) {
        end();
    }

    void eventHandler(const ui::MouseLeave&) {
        end();
        system->setMouseCursorVisible(true);
    }

    void eventHandler(const ui::MouseMove& event) {
        paint(event.globalX - canvas->globalRect.x, event.globalY - canvas->globalRect.y, event.buttons);
        Tool::Preview* preview = nullptr;
        if (activeTool)
            preview = activeTool->getPreview();
        system->setMouseCursorVisible(!preview || !preview->hideCursor);
    }

    void end() {
        prevButtons = ~U32{};
        if (points.empty())
            return;
        if (activeTool)
            activeTool->end(activeCell->getComposite(), points);
        points.clear();
    }

    void paint(S32 tx, S32 ty, U32 buttons) {
        if (prevButtons != buttons)
            end();

        prevButtons = buttons;
        auto rect = canvas->globalRect;
        if (!rect.width || !rect.height)
            return;

        auto surface = activeCell->getComposite();
        S32 x = tx * S32(surface->width()) / S32(rect.width);
        S32 y = ty * S32(surface->height()) / S32(rect.height);
        S32 z = msg::MouseMove::pressure * 255;
        bool begin = points.empty();
        if (!begin && x == points.back().x && y == points.back().y)
            return;
        points.push_back({x, y, z});

        if (begin) {
            do {
                activeTool = Tool::active.lock();
                if (!activeTool)
                    return;
                activeTool->begin(surface, points, buttons);
            } while (Tool::active.lock() != activeTool);
        } else if (activeTool) {
            activeTool->update(surface, points);
        }
    }

    void eventHandler(const ui::MouseWheel& event) {
        inject<Command> zoom{"zoom"};
        zoom->set("level", "*" + tostring(1 + 0.2 * event.wheelY));
        zoom->run();
    }

    void rezoom() {
        if (!*doc || !canvas)
            return;
        container->load({
                {"x", "center"},
                {"y", "center"},
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
        if (filePath->empty()) {
            logI("Empty file path");
            return;
        }

        auto file = FileSystem::parse(filePath);
        if (file.has<std::nullptr_t>()) {
            logE("Could not parse ", filePath);
            return;
        }

        std::shared_ptr<Document> newDoc;
        if (file.has<std::shared_ptr<Document>>()) {
            newDoc = file.get<std::shared_ptr<Document>>();
        } else {
            newDoc = inject<Document>{"new"};
            if (!newDoc->load(file)) {
                return;
            }
        }
        *doc = newDoc;
        newDoc->setPath(filePath);
        showFile();
    }

    void showFile() {
        node()->load({
                {"visible", true},
                {"doc", *doc}
            });
        setFrame();
        activate();
        node()->focus();
        rezoom();
    }

    void activate() {
        editorProvides.emplace(node(), "activeeditor");
        docProvides.emplace(doc->get(), "activedocument");
        pub(msg::ActivateEditor{node()});
        pub(msg::ActivateDocument{doc});
        pub(msg::ActivateCell{activeCell});
    }

    void eventHandler(const ui::FocusChild&) {activate();}
    void eventHandler(const ui::Focus&) {activate();}

    void on(msg::ActivateTool&) {
        end();
    }

    void on(msg::ResizeDocument& msg) {
        if (msg.doc.get() == doc->get()) {
            rezoom();
        }
    }

    void on(msg::ActivateDocument& msg) {
        if (msg.doc.get() != doc->get()) {
            docProvides.reset();
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
