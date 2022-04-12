// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <memory>

#include <cmd/Command.hpp>
#include <common/Surface.hpp>
#include <doc/BitmapCell.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <filters/Filter.hpp>
#include <tools/Tool.hpp>

class CanvasResize : public Filter {
public:
    Property<U32> width{this, "width"};
    Property<U32> height{this, "height"};
    Property<std::shared_ptr<Document>> doc{this, "document"};
    Rect bounds;

    String category() override {return "resize";}

    bool forceAllLayers() override {return true;}
    bool forceAllFrames() override {return true;}

    void updateBounds() {
        bounds.clear();
        if (*doc) {
            if (auto timeline = (*doc)->currentTimeline()) {
                if (auto cell = std::dynamic_pointer_cast<BitmapCell>(timeline->getCell())) {
                    if (auto selection = cell->getSelection()) {
                        bounds = selection->getTrimmedBounds();
                    }
                }
            }
        }
    }

    std::shared_ptr<PropertySet> getMetaProperties() override {
        if (!bounds.empty())
            return nullptr;

        auto meta = Filter::getMetaProperties();

        inject<Document> doc{"activedocument"};
        if (doc) {
            width.value = doc->width();
            height.value = doc->height();
        }

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "number"},
                    {"label", width.name},
                    {"value", width.value}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "number"},
                    {"label", height.name},
                    {"value", height.value}
                }));

        return meta;
    }

    void undo() override {
        S32 width = undoData->get<S32>("width");
        S32 height = undoData->get<S32>("height");
        (*doc)->setDocumentSize(width, height);
    }

    void beforeRun() override {
        updateBounds();
    }

    void run(std::shared_ptr<Surface> surface) override {
        auto data = surface->getPixels();
        S32 inwidth = surface->width();
        S32 inheight = surface->height();

        auto bounds = this->bounds;

        if (bounds.empty()) {
            bounds = Rect{
                0, 0,
                this->width, this->height
            };
        }

        if (bounds.empty() || bounds == surface->rect())
            return;

        if (inwidth == bounds.width && inheight == bounds.height)
            return;

        if (!undoData) {
            undoData = std::make_shared<PropertySet>();
            undoData->set("width", inwidth);
            undoData->set("height", inheight);
        }

        surface->resize(bounds.width, bounds.height);
        U32 fill = Tool::color.toU32();
        S32 max = std::min<S32>(bounds.width, inwidth);

        for (S32 y = bounds.y; y < bounds.bottom(); ++y) {
            if (U32(y) >= inheight) {
                for (S32 x = 0; x < bounds.width; ++x)
                    surface->setPixelUnsafe(x, y - bounds.y, fill);
            } else {
                U32 yi = y * inwidth;
                S32 x = bounds.x;
                S32 xend = std::min<S32>(bounds.right(), inwidth + bounds.x);
                S32 leftFill = std::min<S32>(bounds.right(), std::max<S32>(0, -bounds.x));
                for (; x < leftFill; ++x)
                    surface->setPixelUnsafe(x - bounds.x, y - bounds.y, fill);
                for (; x < xend; ++x)
                    surface->setPixelUnsafe(x - bounds.x, y - bounds.y, data[yi + x]);
                for (; x < bounds.right(); ++x)
                    surface->setPixelUnsafe(x - bounds.x, y - bounds.y, fill);
            }
        }

        surface->setDirty(surface->rect());
        (*doc)->setDocumentSize(bounds.width, bounds.height);
    }

    void afterRun() override {
        auto historyLock = (*doc)->getHistoryLock();
        inject<Command> selectNone {"selectnone"};
        if (selectNone)
            selectNone->run();
    }
};

static Filter::Shared<CanvasResize> reg{"canvas resize"};
