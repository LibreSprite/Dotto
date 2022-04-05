// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef NO_FREETYPE

#ifdef EMSCRIPTEN
#include <config/ftheader.h>
#else
#include <ft2build.h>
#endif
#include FT_FREETYPE_H

#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <common/Font.hpp>
#include <log/Log.hpp>

using namespace fs;

class Glyph {
    Vector<U8> data;

public:
    U32 width, height;
    S32 bearingX, bearingY;
    S32 advance;

    Glyph(FT_Face face) {
        data.resize(face->glyph->bitmap.width * face->glyph->bitmap.rows);
        bearingX = face->glyph->bitmap_left;
        bearingY = face->glyph->bitmap_top;
        advance = face->glyph->advance.x >> 6;
        width = face->glyph->bitmap.width;
        U32 pitch = face->glyph->bitmap.pitch;
        height = face->glyph->bitmap.rows;
        for (U32 y = 0; y < height; ++y) {
            for (U32 x = 0; x < width; ++x) {
                data[y * width + x] = face->glyph->bitmap.buffer[y * pitch + x];
            }
        }
    }

    void blitTo(S32& offsetX, S32& offsetY, const Color& color, Surface& target) {
        for (U32 y = 0; y < height; ++y) {
            for (U32 x = 0; x < width; ++x) {
                auto alpha = data[y * width + x];
                target.setPixel(x + offsetX + bearingX,
                                y + offsetY - bearingY,
                                Color(color.r, color.g, color.b, alpha));
            }
        }
        offsetX += advance;
    }
};

class TTFFont : public Font {
public:
    static FT_Library init() {
        static FT_Library ft = nullptr;
        static FT_Error error = 0;
        if (ft || error)
            return ft;

        error = FT_Init_FreeType(&ft);
        if (error != 0) {
            logE("Failed to initialize FreeType");
        }
        return ft;
    }

    FT_Face face = nullptr;
    U32 currentSize = ~U32{};
    HashMap<FT_UInt, std::shared_ptr<Glyph>> glyphCache;
    Vector<FT_Byte> faceData;

    TTFFont(File& file) {
        auto ft = init();
        if (!ft)
            return;

        faceData.resize(file.size());
        file.read(&faceData[0], faceData.size());

        auto err = FT_New_Memory_Face(ft, &faceData[0], faceData.size(), 0, &face);
        if (err != 0) {
            logE("Failed to load face ", file.getUID());
        }
    }

    FT_UInt getGlyphIndex(const String& text, U32& offset, U32& glyph) {
        glyph = text[offset];
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
        return FT_Get_Char_Index(face, glyph); // TODO: Harfbuzz
    }

    void setSize(U32 size) {
        if (size == currentSize)
            return;
        auto err = FT_Set_Pixel_Sizes(face, 0, size);
        if (err != 0) {
            logE("Failed to set pixel size");
        } else {
            currentSize = size;
            glyphCache.clear();
        }
    }

    Glyph* loadGlyph(const String& text, U32& offset) {
        U32 utf8 = 0;
        FT_UInt glyphIndex = getGlyphIndex(text, offset, utf8);
        auto it = glyphCache.find(glyphIndex);
        if (it != glyphCache.end())
            return it->second.get();

        FT_Int32 load_flags = FT_LOAD_DEFAULT;
        auto err = FT_Load_Glyph(face, glyphIndex, load_flags);
        if (err != 0) {
            logE("Failed to load glyph\n");
            return nullptr;
        }

        err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if (err != 0) {
            logE("Failed to render the glyph\n");
            return nullptr;
        }

        auto glyph = std::make_shared<Glyph>(face);
        glyphCache[glyphIndex] = glyph;
        return glyph.get();
    }

    virtual std::shared_ptr<Surface> print(U32 size, const Color& color, const String& text, const Rect& padding, Vector<S32>& advance) {
        advance.clear();
        if (text.empty())
            return nullptr;
        auto surface = std::make_shared<Surface>();
        setSize(size);
        Vector<Glyph*> glyphs;
        U32 width = 0;
        U32 height = 0;
        for (U32 i = 0, len = text.size(); i < len; ++i) {
            if (auto glyph = loadGlyph(text, i)) {
                glyphs.push_back(glyph);
                advance.push_back(glyph->advance);
                width += glyph->advance;
                height = std::max(height, size + (glyph->height - glyph->bearingY));
            }
        }
        if (!advance.empty()) {
            advance[0] += padding.x;
        }
        surface->resize(width + padding.x + padding.width, height + padding.y + padding.height);
        S32 x = padding.x, y = size + padding.y;
        for (auto glyph : glyphs)
            glyph->blitTo(x, y, color, *surface);
        return surface;
    }
};

class FontParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        auto font = std::make_shared<TTFFont>(*file);
        return font->face ? std::static_pointer_cast<Font>(font) : nullptr;
    }
};

static Parser::Shared<FontParser> font{"ttf"};

#endif
