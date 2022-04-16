// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <doc/Cell.hpp>

class GroupCell  : public Cell {
public:
    virtual U32 layerCount() const = 0;
    virtual std::shared_ptr<Cell> getCell(U32 layer) const = 0;
    virtual void setCell(U32 layer, std::shared_ptr<Cell> cell) = 0;
    virtual void resize(U32 count) = 0;
    virtual bool empty() = 0;
};
