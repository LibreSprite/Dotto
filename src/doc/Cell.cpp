// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <doc/Cell.hpp>

void Cell::modify(bool silent) {
    if (!silent) {
        PubSub<>::pub(msg::ModifyCell{shared_from_this()});
    }
}

void Cell::setAlpha(F32 v, bool silent) {
    auto old = alpha;
    alpha = std::clamp(v, 0.0f, 1.0f);
    modify(old == alpha || silent);
}

void Cell::setBlendMode(const String& newMode, bool silent) {
    if (blendMode == newMode)
        return;
    blendMode = newMode;
    modify(silent);
}

class BitmapCell  : public Cell {
public:
    String getType() const override {return "bitmap";}

    Vector<U8> serialize() override {
        return {};
    }

    bool unserialize(const Vector<U8>&) override {
        return true;
    }
};

static Cell::Shared<BitmapCell> reg{"bitmap"};
