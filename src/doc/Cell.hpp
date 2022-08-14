// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>
#include <common/Surface.hpp>
#include <doc/Selection.hpp>

class Document;
class Timeline;

class Cell  : public Injectable<Cell>, public Serializable, public std::enable_shared_from_this<Cell> {
protected:
    friend class Document;
    String GUID;
    String blendMode = "normal";
    String name;
    std::shared_ptr<Surface> composite = std::make_shared<Surface>();
    std::shared_ptr<Selection> mask;
    F32 alpha = 1.0f;
    Cell* _parent = nullptr;
    Document* _document = nullptr;

    void modify(bool silent);

public:
    virtual void setParent(Cell* parent) {_parent = parent;}
    Cell* parent() const {return _parent;}

    virtual void setDocument(Document* document);
    Document* document() const {return _document;}

    virtual String getType() const = 0;
    const String& getGUID() {return GUID;}
    virtual Surface* getComposite() {return composite.get();}
    Selection* getMask() {return mask.get();}

    virtual Vector<U8> serialize() = 0;
    virtual bool unserialize(const Vector<U8>&) = 0;

    const String& getName() {return name;}
    void setName(const String&, bool silent);

    F32 getAlpha() {return alpha;}
    void setAlpha(F32 v, bool silent);

    const String& getBlendMode() {return blendMode;}
    void setBlendMode(const String&, bool silent);

    virtual void setSelection(const Selection* selection) = 0;
};
