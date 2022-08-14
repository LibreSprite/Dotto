// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>
#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <doc/Cell.hpp>
#include <doc/GroupCell.hpp>

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

class GroupCellImpl  : public GroupCell {
public:
    class ChildCell {
    public:
        std::shared_ptr<Cell> cell;
        fork_ptr<Texture> watcher;
        ChildCell(std::shared_ptr<Cell> cell) : cell{cell} {}
    };

    PubSub<msg::ModifyCell> pub{this};
    Vector<std::shared_ptr<ChildCell>> data;
    HashMap<String, std::shared_ptr<Blender>> blenders;
    std::shared_ptr<Surface> previousResult;
    static inline std::shared_ptr<Surface> tmp = std::make_shared<Surface>();

    String getType() const override {return "group";}

    Vector<U8> serialize() override {
        return {};
    }

    void setSelection(const Selection* selection) override {}

    bool unserialize(const Vector<U8>&) override {
        return true;
    }

    U32 layerCount() const override {
        return data.size();
    }

    void resize(U32 count) override {
        if (count < data.size()) {
            for (std::size_t i = count, max = data.size(); i < max; ++i) {
                if (data[i] && data[i]->cell)
                    data[i]->cell->setParent(nullptr);
            }
        }
        data.resize(count);
    }

    bool empty() override {
        return data.empty();
    }

    std::shared_ptr<Cell> getCell(U32 layer) const override {
        return data.size() > layer && data[layer] ? data[layer]->cell : nullptr;
    }

    void setCell(U32 layer, std::shared_ptr<Cell> cell) override {
        if (!cell) {
            if (layer >= layerCount())
                return;

            if (data[layer] && data[layer]->cell) {
                data[layer]->cell->setParent(nullptr);
                data[layer]->cell->setDocument(nullptr);
            }

            data[layer].reset();
        } else {
            if (layer >= layerCount())
                resize(layer + 1);

            if (data[layer] && data[layer]->cell)
                data[layer]->cell->setParent(nullptr);

            data[layer] = cell ? std::make_shared<ChildCell>(cell) : nullptr;

            if (cell) {
                cell->setParent(this);
                cell->setDocument(document());
            }
        }

        previousResult.reset();
        pub(msg::ModifyGroup{});
    }

    void on(msg::ModifyCell& event) {
        for (auto& child : data) {
            if (child->cell == event.cell) {
                previousResult.reset();
                break;
            }
        }
    }

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

    Surface* getComposite() override {
        std::shared_ptr<Surface> low;
        std::shared_ptr<Surface> result;

        auto layers = layerCount();

        if (!layers)
            return composite.get();

        Rect dirty;

        for (auto i = 0; i < layers; ++i) {
            if (!data[i])
                continue;
            auto& cell = data[i]->cell;
            if (!cell) {
                continue;
            }

            auto& watcher = data[i]->watcher;

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
            dirty.intersect(composite->rect());
            region.clear();
        }

        if (dirty.empty() && previousResult) {
            return previousResult.get();
        }

        bool dirtyResult = false;
        result = composite;

        for (auto i = 0; i < layers; ++i) {
            if (!data[i])
                continue;
            auto& watcher = data[i]->watcher;
            auto& cell = data[i]->cell;
            auto composite = cell->getComposite();
            if (!low) {
                result->resize(composite->width(), composite->height());
                tmp->resize(result->width(), result->height());
                result = composite->shared_from_this();
                if (dirty.empty())
                    dirty = result->rect();
            } else {
                auto high = composite;
                F32 alpha = cell->getAlpha();
                result = low == tmp ? this->composite : tmp;
                getBlender(cell->getBlendMode()).blend(result.get(), low.get(), high, alpha, dirty);
                dirtyResult = true;
            }
            low = result;
        }

        if (result == tmp) {
            for (S32 y = dirty.y; y < dirty.bottom(); ++y) {
                for (S32 x = dirty.x; x < dirty.right(); ++x) {
                    composite->setPixelUnsafe(x, y, result->getPixelUnsafe(x, y));
                }
            }
            result = composite;
        }

        if (dirtyResult)
            result->setDirty(dirty);

        previousResult = result;
        return result.get();
    }
};

static Cell::Shared<GroupCellImpl> reg{"group"};
