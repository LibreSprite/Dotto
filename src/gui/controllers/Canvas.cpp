// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <common/fork_ptr.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Texture.hpp>

class DirtyWatcher : public Texture {
public:
    Rect dirtyRegion;
    Cell* cell;

    DirtyWatcher(Cell* cell) : cell{cell} {
        setDirty(cell->getComposite()->rect());
    }

    void setDirty(const Rect& region) override {
        dirtyRegion.expand(region);
    }
};

class Canvas : public ui::Controller {
public:
    PubSub<msg::ModifyCell,
           msg::PostTick> pub{this};

    Property<std::shared_ptr<Surface>> surface{this, "surface"};
    Property<std::shared_ptr<Document>> doc{this, "document", nullptr, &Canvas::setDocument};
    Property<U32> frame{this, "frame", 0, &Canvas::setFrame};
    Property<U32> layer{this, "layer"};

    Vector<fork_ptr<Texture>> watchers;
    HashMap<String, std::shared_ptr<Blender>> blenders;

    std::shared_ptr<Surface> a = std::make_shared<Surface>();
    std::shared_ptr<Surface> b = std::make_shared<Surface>();
    std::shared_ptr<Cell> activeCell;

    Blender& getBlender(const String& name) {
        auto it = blenders.find(name);
        if (it != blenders.end())
            return *it->second;
        std::shared_ptr<Blender> blender;
        blender = inject<Blender>{name};
        if (!blender)
            blender = inject<Blender>{"normal"};
        blenders[name] = blender;
        return *blender;
    }

    void setDocument() {
        setFrame();
    }

    void setFrame() {
        watchers.clear();
        redraw(true);
    }

    void redraw(bool forceFullRedraw) {
        if (!*doc) {
            return;
        }
        auto& doc = **this->doc;
        auto timeline = doc.currentTimeline();
        auto frames = timeline->frameCount();
        if (frame >= frames) {
            logI("Skipping redraw because frame >= frames");
            return;
        }
        auto layers = timeline->layerCount();
        if (!layers) {
            logI("Skipping redraw because !layers");
            return;
        }

        activeCell = timeline->getCell(frame, layer);

        Surface* low = nullptr;
        Surface* result = nullptr;

        watchers.resize(layers);

        Rect dirty;

        for (auto i = 0; i < layers; ++i) {
            auto& watcher = watchers[i];
            auto cell = timeline->getCell(frame, i);
            if (!cell) {
                continue;
            }
            auto composite = cell->getComposite();
            if (!composite) {
                continue;
            }

            if (!watcher || watcher.shared() != composite->info().get<Texture>(this)) {
                watcher.emplace<DirtyWatcher>(cell.get());
                composite->info().set(this, watcher);
            }

            auto& region = dynamic_cast<DirtyWatcher*>(watcher.get())->dirtyRegion;
            dirty.expand(region);
            region.clear();
        }

        if (forceFullRedraw) {
            dirty = activeCell->getComposite()->rect();
        } else {
            dirty.intersect(activeCell->getComposite()->rect());
        }

        if (!dirty.empty()) {

            bool dirtyResult = false;
            for (auto i = 0; i < layers; ++i) {
                if (!watchers[i])
                    continue;
                auto cell = static_cast<DirtyWatcher*>(watchers[i].get())->cell;
                auto composite = cell->getComposite();
                if (!low) {
                    result = composite;
                    a->resize(result->width(), result->height());
                    b->resize(result->width(), result->height());
                } else {
                    auto high = composite;
                    F32 alpha = cell->getAlpha();
                    result = low == a.get() ? b.get() : a.get();
                    getBlender(cell->getBlendMode()).blend(result, low, high, alpha, dirty);
                    dirtyResult = true;
                }
                low = result;
            }

            if (dirtyResult)
                result->setDirty(dirty);
            if (result)
                node()->load({{"surface", result->shared_from_this()}});
        }
    }

    void on(msg::ModifyCell& event) {
        redraw(true);
    }

    void on(msg::PostTick&) {
        redraw(false);
    }

    void attach() override {
        redraw(true);
    }
};

static ui::Controller::Shared<Canvas> canvas{"canvas"};
