// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Cell.hpp>

class BitmapCell  : public Cell {
public:
    Vector<U8> serialize() override {
        return {};
    }

    bool unserialize(const Vector<U8>&) override {
        return true;
    }
};

static Cell::Shared<BitmapCell> reg{"bitmap"};
