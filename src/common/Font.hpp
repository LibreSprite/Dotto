// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Rect.hpp>
#include <common/Surface.hpp>

class Font {
public:
    virtual std::shared_ptr<Surface> print(U32 size, const Color& color, const String& text, const Rect& padding, Vector<S32>& advance);

    class Glyph {
    public:
        virtual ~Glyph() = default;
        S32 advance;
        S32 bearingX, bearingY;
        U32 width, height;
        Vector<U8> data;
        virtual void blitTo(S32& offsetX, S32& offsetY, const Color& color, Surface& target, U8 threshold = 0);
    };

    virtual void setSize(U32 size) = 0;
    virtual Glyph* loadGlyph(const String& text, U32& offset) = 0;

protected:
    U32 getGlyph(const String& text, U32& offset);
    U32 currentSize = ~U32{};
    HashMap<U32, std::shared_ptr<Glyph>> glyphCache;
};
