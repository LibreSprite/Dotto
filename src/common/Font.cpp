// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "Font.hpp"

void Font::Glyph::blitTo(S32& offsetX, S32& offsetY, const Color& color, Surface& target, U8 threshold) {
    if (threshold == 0) {
        for (U32 y = 0; y < height; ++y) {
            for (U32 x = 0; x < width; ++x) {
                auto alpha = data[y * width + x];
                target.setPixel(x + offsetX + bearingX,
                                y + offsetY - bearingY,
                                Color(color.r, color.g, color.b, alpha));
            }
        }
    } else {
        for (U32 y = 0; y < height; ++y) {
            for (U32 x = 0; x < width; ++x) {
                if (data[y * width + x] < threshold)
                    continue;
                target.setPixel(x + offsetX + bearingX,
                                y + offsetY - bearingY,
                                color);
            }
        }
    }
    offsetX += advance;
}

std::shared_ptr<Surface> Font::print(U32 size, const Color& color, const String& text, const Rect& padding, Vector<S32>& advance) {
    advance.clear();
    if (text.empty())
        return nullptr;
    auto surface = std::make_shared<Surface>();
    setSize(size);

    struct Entity {
        Glyph* glyph;
        Color color;
        bool hasAdvance;
    } entity;
    entity.color = color;
    entity.hasAdvance = true;
    Vector<Entity> glyphs;

    U32 width = 0;
    U32 height = 0;
    U32 escapeDepth = 0;
    U32 escapeStart = 0;
    U32 escapeEnd = 0;
    U32 maxWidth = 0;
    for (U32 i = 0, len = text.size(); i < len; ++i) {
        if (escapeDepth) {
            if (text[i] == '[') {
                escapeDepth++;
            } else if (text[i] == ']') {
                escapeDepth--;
                if (escapeDepth == 1) {
                    escapeDepth = 0;
                    escapeEnd = i - 1;
                    auto code = std::string_view{text}.substr(escapeStart, escapeEnd - escapeStart + 1);
                    if (code == "zw") {
                        entity.hasAdvance = false;
                    } else if (code == "w" ) {
                        entity.hasAdvance = true;
                    } else if (code == "r") {
                        entity.hasAdvance = true;
                        entity.color = color;
                    } else {
                        entity.color.fromString(String{code});
                    }
                }
            }
            continue;
        }
        if (text[i] == '\x1B') {
            escapeDepth = 1;
            escapeStart = i + 2;
            continue;
        }
        if (auto glyph = loadGlyph(text, i)) {
            entity.glyph = glyph;
            glyphs.push_back(entity);
            advance.push_back(entity.hasAdvance ? glyph->advance : 0);
            maxWidth = std::max(maxWidth, width + glyph->advance);
            if (entity.hasAdvance) {
                width += glyph->advance;
            }
            height = std::max(height, size + (glyph->height - glyph->bearingY));
        }
    }
    width = std::max(maxWidth, width);
    if (!advance.empty()) {
        advance[0] += padding.x;
    }
    surface->resize(width + padding.x + padding.width, height + padding.y + padding.height);
    S32 x = padding.x, y = size + padding.y;
    for (auto& entity : glyphs) {
        entity.glyph->blitTo(x, y, entity.color, *surface);
    }
    return surface;
}

U32 Font::getGlyph(const String& text, U32& offset) {
    U32 glyph = text[offset];
    U32 extras = 0;
    if (glyph & 0b1000'0000) {
        U32 max = text.size();
        if (!(glyph & 0b0010'0000)) { // 110xxxxx 10xxxxxx
            glyph &= 0b11111;
            extras = 1;
        } else if (!(glyph & 0b0001'0000)) { // 1110xxxx 10xxxxxx 10xxxxxx
            glyph &= 0b1111;
            extras = 2;
        } else { // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            glyph &= 0b111;
            extras = 3;
        }
        for (U32 i = 0; i < extras; ++i) {
            if (++offset < max) {
                glyph <<= 6;
                glyph |= text[offset] & 0b111111;
            }
        }
    }
    return glyph;
}
