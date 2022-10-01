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
    Vector<FT_Byte> faceData;

    TTFFont(File& file) {
        auto ft = init();
        if (!ft)
            return;

        faceData.resize(file.size());
        file.read(faceData.data(), faceData.size());

        auto err = FT_New_Memory_Face(ft, &faceData[0], faceData.size(), 0, &face);
        if (err != 0) {
            logE("Failed to load face ", file.getUID());
        }
    }

    void setSize(U32 size) override {
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

    Glyph* loadGlyph(U32 utf8) override {
        U32 glyphIndex = FT_Get_Char_Index(face, utf8);
        if (!glyphIndex)
            return nullptr;

        if (auto it = glyphCache.find(glyphIndex); it != glyphCache.end())
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

        auto glyph = std::make_shared<Glyph>();
        glyphCache[glyphIndex] = glyph;

        glyph->data.resize(face->glyph->bitmap.width * face->glyph->bitmap.rows);
        glyph->bearingX = face->glyph->bitmap_left;
        glyph->bearingY = face->glyph->bitmap_top;
        glyph->advance = face->glyph->advance.x >> 6;
        glyph->width = face->glyph->bitmap.width;
        U32 pitch = face->glyph->bitmap.pitch;
        glyph->height = face->glyph->bitmap.rows;
        for (U32 y = 0; y < glyph->height; ++y) {
            for (U32 x = 0; x < glyph->width; ++x) {
                glyph->data[y * glyph->width + x] = face->glyph->bitmap.buffer[y * pitch + x];
            }
        }

        return glyph.get();
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
