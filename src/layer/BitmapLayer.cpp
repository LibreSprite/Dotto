// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/System.hpp>
#include <doc/BitmapCell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <layer/Layer.hpp>
#include <tools/Tool.hpp>

class BitmapLayer : public Layer {
    PubSub<msg::ActivateTool,
           msg::PreModifySelection,
           msg::Tick> pub{this};
    U32 prevButtons = ~U32{};
    std::shared_ptr<Tool> activeTool;
    Tool::Path points;
    inject<System> system;
    std::optional<Selection::Provides> mask;
    Tool::Preview preview {
        .hideCursor = false,
        .overlay = inject<Selection>{"new"}
    };

    Rect selectionGlobalCanvas;
    F32 selectionScale;

public:
    U32 frameCounter = 0;
    void on(msg::Tick&) {
        if (frameCounter++ < 10)
            return;
        frameCounter = 0;
        Tool::antAge++;

        if (preview.draw == Tool::Preview::drawOutlineAnts)
            preview.draw(false, preview, *overlayLayer(), offsetCanvas(), overlayScale());

        clearSelectionOverlay();

        auto cell = dynamic_cast<BitmapCell*>(&this->cell());
        if (!cell)
            return;

        auto selection = cell->getSelection();
        if (!selection || selection->empty()) {
            this->selection.reset();
            return;
        }

        this->selection = selection->shared_from_this();
        selectionGlobalCanvas = offsetCanvas();
        selectionScale = overlayScale();

        Tool::Preview preview {
            .overlay = this->selection,
            .overlayColor = Color{200, 200, 200, 200},
            .altColor = Color{55, 55, 55, 200},
            .draw = Tool::Preview::drawOutlineAnts
        };
        Tool::Preview::drawOutlineAnts(false, preview, *overlayLayer(), selectionGlobalCanvas, selectionScale);
    }

    void on(msg::ActivateTool&) {end();}

    void update() override {
        if (prevButtons != buttons())
            end();

        prevButtons = buttons();
        if (globalCanvas().empty())
            return;

        auto cell = dynamic_cast<BitmapCell*>(&this->cell());
        if (!cell)
            return;

        auto point = localMouse();

        auto surface = cell->getComposite();
        if (auto selection = cell->getSelection()) {
            mask.emplace(selection, "active");
        } else {
            mask.reset();
        }

        bool begin = points.empty();
        if (!begin && point.x == points.back().x && point.y == points.back().y)
            return;

        if (!buttons() && points.size() > 5) {
            points.erase(points.begin(), points.begin() + (points.size() - 5));
        }

        points.push_back(point);

        if (begin) {
            do {
                activeTool = Tool::active.lock();
                if (!activeTool)
                    return;
                activeTool->begin(surface, points, buttons());
            } while (Tool::active.lock() != activeTool);
        } else if (activeTool) {
            activeTool->update(surface, points);
        }

        updateToolOverlay();
    }

    void end() {
        prevButtons = ~U32{};
        if (points.empty())
            return;
        if (activeTool)
            activeTool->end(cell().getComposite(), points);
        points.clear();
    }

    void clearOverlays() override {
        clearToolOverlay();
        clearSelectionOverlay();
    }

    F32 overlayScale() {
        return F32(globalCanvas().width) / F32(localCanvas().width);
    }

    void clearToolOverlay() {
        if (!preview.overlay->empty()) {
            preview.draw(true, preview, *overlayLayer(), offsetCanvas(), overlayScale());
            preview.overlay->clear();
        }
    }

    void clearSelectionOverlay() {
        if (selection) {
            Tool::Preview preview {.overlay = selection};
            Tool::Preview::drawOutlineSolid(true, preview, *overlayLayer(), selectionGlobalCanvas, selectionScale);
            selection.reset();
        }
    }

    std::shared_ptr<Selection> selection = nullptr;

    void on(msg::PreModifySelection& event) {
        if (event.selection == selection.get()) {
            clearSelectionOverlay();
        }
    }

    void updateToolOverlay() {
        if (!overlayLayer())
            return;

        Tool::Preview* currentPreview = nullptr;
        if (auto activeTool = Tool::active.lock())
            currentPreview = activeTool->getPreview();

        clearToolOverlay();

        if (currentPreview) {
            if (currentPreview->overlay) {
                *preview.overlay = *currentPreview->overlay;
            }
            preview.overlayColor = currentPreview->overlayColor;
            preview.altColor = currentPreview->altColor;
            preview.hideCursor = currentPreview->hideCursor;
            preview.draw = currentPreview->draw;
        } else {
            preview.hideCursor = false;
        }

        if (!preview.overlay->empty()) {
            preview.draw(false,
                         preview,
                         *overlayLayer(),
                         offsetCanvas(),
                         overlayScale());
        }

        system->setMouseCursorVisible(!preview.hideCursor);
    }
};

static Layer::Shared<BitmapLayer> reg{"bitmap"};
