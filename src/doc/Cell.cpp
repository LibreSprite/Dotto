// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Cell.hpp>

class CellImpl  : public Cell {
public:
    CellImpl() {
        composite = std::make_shared<Surface>();
        composite->resize(1024, 1024);
        memset(composite->data(), 0xFF, composite->dataSize());
    }

    Vector<U8> serialize() override {
        return {};
    }

    bool unserialize(const Vector<U8>&) override {
        return true;
    }
};

static Cell::Shared<CellImpl> temporary{"temporary"};
