// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "Font.hpp"
#include <variant>

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

Vector<Font::Entity> Font::parse(std::string_view text) {
    Vector<Font::Entity> out;
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
                    auto code = text.substr(escapeStart, escapeEnd - escapeStart + 1);
                    if (code == "zw") {
                        out.push_back(Command::NoAdvance);
                    } else if (code == "w" ) {
                        out.push_back(Command::Advance);
                    } else if (code == "r") {
                        out.push_back(Command::Reset);
                    } else {
                        out.push_back(Color(String{code}));
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
        if (auto glyph = getUTF8(text, i)) {
            out.push_back(glyph);
        }
    }
    return out;
}

std::shared_ptr<Surface> Font::print(U32 size, const Color& color, const String& text, const Rect& padding, Vector<S32>& advance) {
    advance.clear();
    if (text.empty())
        return nullptr;
    auto surface = std::make_shared<Surface>();
    setSize(size);

    auto entities = parse(text);;
    Vector<Glyph*> glyphs;
    glyphs.reserve(entities.size());

    Color fgColor = color;
    bool hasAdvance = true;
    U32 width = 0;
    U32 maxWidth = 0;
    U32 height = 0;
    for (auto& entity : entities) {
        if (auto index = std::get_if<U32>(&entity); index && *index) {
            if (auto glyph = loadGlyph(*index)) {
                advance.push_back(hasAdvance ? glyph->advance : 0);
                maxWidth = std::max(maxWidth, width + glyph->advance);
                if (hasAdvance) {
                    width += glyph->advance;
                }
                height = std::max(height, size + (glyph->height - glyph->bearingY));
                glyphs.push_back(glyph);
                continue;
            }
        }else if (auto cmd = std::get_if<Command>(&entity)) {
            if (*cmd == Command::Advance) {
                hasAdvance = true;
            } else if (*cmd == Command::NoAdvance) {
                hasAdvance = false;
            }
        }
        glyphs.push_back(nullptr);
    }

    hasAdvance = true;
    if (!advance.empty()) {
        advance[0] += padding.x;
    }

    width = std::max(maxWidth, width);
    surface->resize(width + padding.x + padding.width, height + padding.y + padding.height);

    S32 x = padding.x, y = size + padding.y;
    for (U32 max = entities.size(), i = 0; i < max; ++i) {
        if (auto glyph = glyphs[i]) {
            glyph->blitTo(x, y, fgColor, *surface);
        } else if (auto newColor = std::get_if<Color>(&entities[i])) {
            fgColor = *newColor;
        } else if (auto cmd = std::get_if<Command>(&entities[i])) {
            switch (*cmd) {
            case Command::Advance:
                hasAdvance = true;
                break;
            case Command::NoAdvance:
                hasAdvance = false;
                break;
            case Command::Reset:
                hasAdvance = true;
                fgColor = color;
                break;
            }
        }
    }

    return surface;
}

U32 Font::getUTF8(std::string_view text, U32& offset) {
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
