// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>

class Cell;
class DocumentImpl;

class Timeline : public Injectable<Timeline>, public Serializable, public std::enable_shared_from_this<Timeline> {
    friend class DocumentImpl;
    String GUID;

public:
    virtual U32 frameCount() const = 0;
    virtual U32 layerCount() const = 0;
    virtual U32 frame() const = 0;
    virtual U32 layer() const = 0;
    virtual std::shared_ptr<Cell> activate(U32 frame, U32 layer) = 0;
    virtual std::shared_ptr<Cell> getCell() const {return getCell(frame(), layer(), false);}
    virtual std::shared_ptr<Cell> getCell(U32 frame, U32 layer, bool loop = false) const = 0;
    virtual void setCell(U32 frame, U32 layer, std::shared_ptr<Cell> cell) = 0;
};
