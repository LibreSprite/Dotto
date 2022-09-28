// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if defined(USE_STBTTF)

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype/stb_truetype.h>

#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <common/Font.hpp>
#include <log/Log.hpp>

using namespace fs;

class TTFFont : public Font {
public:

    stbtt_fontinfo font;
    bool OK = false;
    Vector<U8> faceData;

    TTFFont(File& file) {
        faceData.resize(file.size());
        file.read(&faceData[0], faceData.size());

        OK = stbtt_InitFont(&font, &faceData[0], 0); // stbtt_GetFontOffsetForIndex(&faceData[0], 0)
        if (!OK) {
            logE("Failed to load face ", file.getUID());
        }
    }

    U32 getGlyphIndex(const String& text, U32& offset) {
        U32 glyph = getGlyph(text, offset);
        return stbtt_FindGlyphIndex(&font, glyph);
    }

    F32 scale = 1.0f;
    void setSize(U32 size) override {
        if (size == currentSize)
            return;
        scale = stbtt_ScaleForMappingEmToPixels(&font, size);
        currentSize = size;
        glyphCache.clear();
    }

    Glyph* loadGlyph(const String& text, U32& offset) {
        U32 glyphIndex = getGlyphIndex(text, offset);
        if (glyphIndex == 0)
            return nullptr;

        auto it = glyphCache.find(glyphIndex);
        if (it != glyphCache.end())
            return it->second.get();

        int advance, lsb;
        stbtt_GetGlyphHMetrics(&font, glyphIndex, &advance, &lsb);

        int x, y, w, h;
        auto bitmap = stbtt_GetGlyphBitmap(&font, scale, scale, glyphIndex, &w, &h, &x, &y);

        auto glyph = std::make_shared<Glyph>();
        glyphCache[glyphIndex] = glyph;

        glyph->data.resize(w * h);
        glyph->bearingX = x;
        glyph->bearingY = -y;
        glyph->advance = advance * scale + 0.5f;
        glyph->width = w;
        glyph->height = h;

        if (bitmap) {
            for (U32 y = 0; y < h; ++y) {
                for (U32 x = 0; x < w; ++x) {
                    glyph->data[y * w + x] = bitmap[y * w + x];
                }
            }
            free(bitmap);
        }

        return glyph.get();
    }
};

class STBTTFFontParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        auto font = std::make_shared<TTFFont>(*file);
        return font->OK ? std::static_pointer_cast<Font>(font) : nullptr;
    }
};

static Parser::Shared<STBTTFFontParser> font{"ttf"};

#endif
