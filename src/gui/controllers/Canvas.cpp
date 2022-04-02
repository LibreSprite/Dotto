// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>
#include <cmd/Command.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <common/System.hpp>
#include <common/fork_ptr.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Texture.hpp>
#include <memory>
#include <tools/Tool.hpp>

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
           msg::ActivateCell,
           msg::ActivateTool,
           msg::PostTick> pub{this};

    Property<std::shared_ptr<Surface>> surface{this, "surface"};
    Property<std::shared_ptr<Document>> doc{this, "document", nullptr, &Canvas::setDocument};
    Property<U32> frame{this, "frame", 0, &Canvas::setFrame};
    Property<U32> layer{this, "layer", 0, &Canvas::setLayer};

    std::shared_ptr<Tool> activeTool;
    Tool::Path points;

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

    void setLayer() {
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

    bool isRelevantCell(std::shared_ptr<Cell> cell) {
        if (!*doc) {
            return false;
        }
        auto& doc = **this->doc;
        auto timeline = doc.currentTimeline();
        auto frames = timeline->frameCount();
        if (frame >= frames) {
            return false;
        }
        auto layers = timeline->layerCount();
        if (!layers) {
            return false;
        }

        for (auto i = 0; i < layers; ++i) {
            if (timeline->getCell(frame, i) == cell)
                return true;
        }
        return false;
    }

    void attach() override {
        node()->addEventListener<ui::MouseMove,
                                 ui::MouseDown,
                                 ui::MouseUp,
                                 ui::MouseWheel,
                                 ui::MouseLeave>(this);
        redraw(true);
    }

    void on(msg::ActivateTool&) {
        end();
    }

    void on(msg::ModifyCell& event) {
        redraw(true);
    }

    void on(msg::ActivateCell& event) {
        node()->set("inputEnabled", isRelevantCell(event.cell));
    }

    void on(msg::PostTick&) {
        redraw(false);
    }

    void eventHandler(const ui::MouseDown& event) {
        paint(event.targetX(), event.targetY(), event.buttons);
    }

    void eventHandler(const ui::MouseUp& event) {
        end();
    }

    void eventHandler(const ui::MouseLeave&) {
        system->setMouseCursorVisible(true);
        end();
    }

    inject<System> system;
    U32 prevButtons = ~U32{};

    void eventHandler(const ui::MouseMove& event) {
        paint(event.targetX(), event.targetY(), event.buttons);

        Tool::Preview* preview = nullptr;
        if (activeTool)
            preview = activeTool->getPreview();
        system->setMouseCursorVisible(!preview || !preview->hideCursor);
    }

    void eventHandler(const ui::MouseWheel& event) {
        inject<Command> zoom{"zoom"};
        zoom->set("level", "*" + tostring(1 + 0.2 * event.wheelY));
        zoom->run();
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
        auto rect = node()->globalRect;
        if (!rect.width || !rect.height)
            return;

        auto surface = activeCell->getComposite();
        S32 x = (tx * surface->width()) / rect.width;
        S32 y = (ty * surface->height()) / rect.height;
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
};

static ui::Controller::Shared<Canvas> canvas{"canvas"};
