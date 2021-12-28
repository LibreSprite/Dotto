// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>
#include <common/Surface.hpp>

class Document;

class Cell  : public Injectable<Cell>, public Serializable, public std::enable_shared_from_this<Cell> {
    friend class Document;
    String type;
    String GUID;
    std::shared_ptr<Surface> composite;

public:
    const String& getType() {return type;}
    const String& getGUID() {return GUID;}
    Surface* getComposite() {return composite.get();}
    virtual Vector<U8> serialize() = 0;
    virtual bool unserialize(const Vector<U8>&) = 0;
};
