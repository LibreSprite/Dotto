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
#include <layer/Layer.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

class Editor : public ui::Controller {
    Property<std::shared_ptr<Document>> doc{this, "doc"};
    PubSub<msg::ResizeDocument,
           msg::ActivateDocument,
           msg::ActivateEditor,
           msg::PollActiveEditor,
           msg::Tick> pub{this};
    std::optional<Document::Provides> docProvides;
    std::optional<ui::Node::Provides> editorProvides;
    Property<String> filePath{this, "file", "", &Editor::openFile};
    Property<std::shared_ptr<PropertySet>> newFileProperties{this, "newfile", nullptr, &Editor::newFile};
    Property<F32> scale{this, "scale", 0.0f, &Editor::rezoom};
    Property<U32> frame{this, "frame", 0, &Editor::setFrame};
    Property<U32> layer{this, "layer", 0, &Editor::setFrame};
    Property<bool> draggable{this, "draggable", false};
    Property<bool> grid{this, "grid", false, &Editor::changeGrid};
    Property<Color> gridColor{this, "grid-color", Color{128, 128, 255, 128}};
    bool needsGridRedraw = false;

    std::shared_ptr<Surface> overlaySurface;
    std::shared_ptr<Surface> gridSurface;

    std::shared_ptr<ui::Node> canvas;
    std::shared_ptr<ui::Node> container;
    std::shared_ptr<ui::Node> gridoverlay;

    std::shared_ptr<Cell> activeCell;
    std::shared_ptr<Layer> layerEditor;
    inject<System> system;

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

        bool frameChanged = timeline->frame() != *frame;
        bool layerChanged = timeline->layer() != *layer;
        auto cell = timeline->activate(frame, layer);
        if (activeCell == cell)
            return;

        if (!activeCell || activeCell->getType() != cell->getType()) {
            layerEditor = inject<Layer>{cell->getType()};
            layerEditor->setLocalCanvas({0, 0, (*doc)->width(), (*doc)->height()});
            layerEditor->setOverlayLayer(overlaySurface);
        }

        layerEditor->setCell(cell);

        activeCell = cell;

        if (frameChanged) {
            pub(msg::ActivateFrame{doc, frame});
        }

        if (layerChanged) {
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
                                 ui::MouseLeave,
                                 ui::Resize>(this);
        pub(msg::ActivateDocument{doc});
        canvas = node()->findChildById("canvas");
        container = node()->findChildById("container");
        if (auto tooloverlay = node()->findChildById("tooloverlay")) {
            overlaySurface = tooloverlay->getPropertySet().get<std::shared_ptr<Surface>>("surface");
        }
        gridoverlay = node()->findChildById("gridoverlay");
        if (gridoverlay) {
            gridSurface = gridoverlay->getPropertySet().get<std::shared_ptr<Surface>>("surface");
        }
    }

    void eventHandler(const ui::Resize&) {
        overlaySurface->resize(node()->globalRect.width, node()->globalRect.height);
        changeGrid();
    }

    void changeGrid() {
        if (*grid && gridSurface && scale >= 3) {
            needsGridRedraw = true;
            gridoverlay->set("visible", true);
        } else if (gridoverlay) {
            gridoverlay->set("visible", false);
        }
    }

    void drawGrid(const Color& color) {
        needsGridRedraw = false;
        gridSurface->resize(node()->globalRect.width, node()->globalRect.height);
        gridSurface->setDirty(gridSurface->rect());
        gridSurface->fillRect(gridSurface->rect(), 0);

        S32 linesY = (*doc)->height();
        S32 linesX = (*doc)->width();
        U32 gridColor = color.toU32();

        for (S32 x = 0; x <= linesX; ++x) {
            gridSurface->setVLine(
                x * scale + canvas->globalRect.x, canvas->globalRect.y,
                canvas->globalRect.height,
                gridColor);
        }

        for (S32 y = 0; y <= linesY; ++y) {
            gridSurface->setHLine(
                canvas->globalRect.x, y * scale + canvas->globalRect.y,
                canvas->globalRect.width,
                gridColor);
        }
    }

    void eventHandler(const ui::MouseDown& event) {
        if (draggable) {
            pub(msg::BeginDrag{
                    container,
                    container->globalRect.x,
                    container->globalRect.y
                });
        } else {
            layerEditor->setGlobalCanvas(canvas->globalRect);
            layerEditor->setGlobalMouse({event.globalX, event.globalY, S32(msg::MouseMove::pressure * 255)});
            layerEditor->setButtons(event.buttons);
            layerEditor->update();
        }
    }

    void eventHandler(const ui::MouseUp& event) {
        layerEditor->setButtons(event.buttons);
        layerEditor->update();
    }

    void eventHandler(const ui::MouseLeave&) {
        system->setMouseCursorVisible(true);
    }

    void eventHandler(const ui::MouseMove& event) {
        layerEditor->setGlobalCanvas(canvas->globalRect);
        layerEditor->setGlobalMouse({event.globalX, event.globalY, S32(msg::MouseMove::pressure * 255)});
        layerEditor->setButtons(event.buttons);
        layerEditor->update();
    }

    void eventHandler(const ui::MouseWheel& event) {
        inject<Command> zoom{"zoom"};
        zoom->set("level", "*" + tostring(1 + 0.2 * event.wheelY));
        zoom->run();
    }

    void rezoom() {
        if (!*doc || !canvas)
            return;

        if (scale == 0) {
            auto editor = node()->globalRect;
            Rect canvas {0, 0, (*doc)->width(), (*doc)->height()};
            if (!editor.empty() && !canvas.empty()) {
                F32 fit = std::min(F32(editor.width) / F32(canvas.width), F32(editor.height) / F32(canvas.height));
                node()->set("scale", fit * 0.9f);
                return;
            }
        }

        if (layerEditor) {
            layerEditor->clearOverlays();
        }

        if (*grid && gridSurface) {
            changeGrid();
        }

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

    void on(msg::ResizeDocument& msg) {
        if (msg.doc.get() == doc->get()) {
            rezoom();
            layerEditor->setLocalCanvas({0, 0, (*doc)->width(), (*doc)->height()});
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

    void on(msg::Tick&) {
        if (needsGridRedraw)
            drawGrid(gridColor);
    }
};

static ui::Controller::Shared<Editor> reg{"editor"};
