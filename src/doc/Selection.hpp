// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Rect.hpp>
#include <common/Surface.hpp>

class Selection : public Injectable<Selection>, public std::enable_shared_from_this<Selection> {
public:
    virtual const Rect& getBounds() const = 0;
    virtual const Vector<U8>& getData() const = 0;
    virtual void add(const Selection& other) = 0;
    virtual void blend(const Selection& other) = 0;
    virtual void add(S32 x, S32 y, U32 amount) = 0;
    virtual void subtract(S32 x, S32 y, U32 amount) = 0;
    virtual U8 get(S32 x, S32 y) = 0;
    virtual Vector<U32> read(Surface*) = 0;
    virtual void write(Surface*, Vector<U32>& pixels) = 0;
    virtual void clear() = 0;
};
