// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>
#include <common/Surface.hpp>
#include <doc/Selection.hpp>

class Document;

class Cell  : public Injectable<Cell>, public Serializable, public std::enable_shared_from_this<Cell> {
protected:
    friend class Document;
    String type;
    String GUID;
    std::shared_ptr<Surface> composite;
    std::shared_ptr<Selection> mask;

public:
    const String& getType() {return type;}
    const String& getGUID() {return GUID;}
    Surface* getComposite() {return composite.get();}
    Selection* getMask() {return mask.get();}
    virtual Vector<U8> serialize() = 0;
    virtual bool unserialize(const Vector<U8>&) = 0;
};
