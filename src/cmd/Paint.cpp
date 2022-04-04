// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>
#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>
#include <doc/Selection.hpp>
#include <log/Log.hpp>
#include <tools/Tool.hpp>

static std::shared_ptr<Surface> backup;
static Surface::PixelType* backupSurface = nullptr;
static std::shared_ptr<Selection> backupSelection;
static Vector<U32> cursorUndo;

class Paint : public Command {
    Property<std::shared_ptr<Selection>> selection{this, "selection"};
    Property<Color> color{this, "color", Tool::color.toString()};
    Property<bool> preview{this, "preview", false};
    Property<bool> cursor{this, "cursor", false};
    Property<String> mode{this, "mode", "normal"};
    Vector<U32> undoData;

public:
    void undo() override {
        restoreBackupSurface();
        auto selection = *this->selection;
        if (!selection) {
            logI("No selection");
            return;
        }
        selection->write(cell()->getComposite(), undoData);
    }

    void setupPreview() {
        auto surface = cell()->getComposite();
        if (!backup) {
            backup = std::make_shared<Surface>();
            backupSelection = inject<Selection>{"new"};
        }

        backup->resize(surface->width(), surface->height());
        auto surfaceData = surface->data();
        if (surfaceData == backupSurface) {
            if (!cursor) {
                backupSelection->blend(*selection->get());
            } else {
                backupSelection->write(surface, cursorUndo);
                backupSelection->clear();
                backupSelection->add(*selection->get());
                cursorUndo = (*selection)->read(backup.get());
            }
            return;
        }

        if (cursor)
            cursorUndo = (*selection)->read(surface);

        backupSelection->clear();
        backupSelection->add(*selection->get());
        backupSurface = surfaceData;
        auto backupData = backup->data();
        for (std::size_t i = 0, size = surface->width() * surface->height(); i < size; ++i) {
            backupData[i] = surfaceData[i];
        }
    }

    void restoreBackupSurface() {
        if (!backup)
            return;
        auto surface = cell()->getComposite();
        auto backupData = backup->data();
        auto surfaceData = surface->data();
        for (std::size_t i = 0, size = surface->width() * surface->height(); i < size; ++i) {
            surfaceData[i] = backupData[i];
        }
        surface->setDirty(surface->rect());
        backup.reset();
        backupSurface = nullptr;
    }

    void run() override {
        auto cell = this->cell();
        if (!cell)
            return;

        auto selection = this->selection->get();
        if (!selection) {
            this->selection.value = inject<Selection>{"new"};
            selection = this->selection->get();
        }
        auto& color = *this->color;
        if (!selection || !color.a) {
            return;
        }

        auto surface = cell->getComposite();
        auto writeData = surface->data();
        auto readData = writeData;

        if (!preview) {
            if (backupSurface == readData) {
                backupSelection->blend(*selection);
                readData = backup->data();
                selection = backupSelection.get();
                std::swap(*this->selection, backupSelection);
                backupSelection->clear();
                undoData = selection->read(backup.get());
            } else {
                undoData = selection->read(surface);
            }
            backupSurface = nullptr;
            if (cursor) {
                restoreBackupSurface();
                return;
            }
        } else {
            setupPreview();
            if (cursor) {
                readData = backup->data();
            }
        }

        auto maskRect = selection->getBounds();

        if (maskRect.empty())
            return;

        auto& mask = selection->getData();
        auto commonRect = maskRect;
        auto maskStride = maskRect.width;
        auto surfaceStride = surface->width();

        commonRect.intersect({0, 0, surface->width(), surface->height()});

        U32 maskOffsetY = commonRect.y > 0 ? 0 : -commonRect.y;
        U32 maskOffsetX = commonRect.x > 0 ? 0 : -commonRect.x;
        U32 surfaceOffsetY = commonRect.y > 0 ? commonRect.y : 0;
        U32 surfaceOffsetX = commonRect.x > 0 ? commonRect.x : 0;

        inject<Blender> blender{*mode};
        if (blender) {
            Color low;
            for (U32 y = 0; y < commonRect.height; ++y) {
                U32 maskIndex = (y + maskOffsetY) * maskStride + maskOffsetX;
                U32 surfaceIndex = (y + surfaceOffsetY) * surfaceStride + surfaceOffsetX;
                for (U32 x = 0; x < commonRect.width; ++x, ++maskIndex, ++surfaceIndex) {
                    auto amount = mask[maskIndex] * (1.0f / 255.0f);
                    low.fromU32(readData[surfaceIndex]);
                    writeData[surfaceIndex] = blender->blendPixel(low, color, amount);
                }
            }
        }

        surface->setDirty(commonRect);
        if (preview)
            this->selection->get()->clear();
        else {
            backup.reset();
            if (!cursor)
                commit();
        }
    }
};

static Command::Shared<Paint> cmd{"paint"};
