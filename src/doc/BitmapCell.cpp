// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/BitmapCell.hpp>

class BitmapCellImpl  : public BitmapCell {
public:
    std::shared_ptr<Selection> selection;

    String getType() const override {return "bitmap";}

    void setSelection(const Selection* selection) override {
        if (!selection || selection->getBounds().width <= 1 || selection->getBounds().height <= 1) {
            this->selection.reset();
        } else {
            if (!this->selection) {
                this->selection = inject<Selection>{"new"};
            }
            *this->selection = *selection;
        }
    }

    Selection* getSelection() override {
        return selection.get();
    }

    Vector<U8> serialize() override {
        return {};
    }

    bool unserialize(const Vector<U8>&) override {
        return true;
    }
};

static Cell::Shared<BitmapCellImpl> reg{"bitmap"};
