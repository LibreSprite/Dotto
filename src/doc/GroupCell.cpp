// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/GroupCell.hpp>

class GroupCellImpl  : public GroupCell {
public:
    Vector<std::shared_ptr<Cell>> data;

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
        data.resize(count);
    }

    bool empty() override {
        return data.empty();
    }

    std::shared_ptr<Cell> getCell(U32 layer) const override {
        return data.size() > layer ? data[layer] : nullptr;
    }

    void setCell(U32 layer, std::shared_ptr<Cell> cell) override {
        if (!cell && layer >= layerCount()) {
            return;
        }
        if (layer >= layerCount())
            resize(layer + 1);
        data[layer] = cell;
    }
};

static Cell::Shared<GroupCellImpl> reg{"group"};
