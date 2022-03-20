// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef NO_FREETYPE

#ifdef USE_SDL1
#include <SDL/SDL_rwops.h>
#include <SDL/SDL_ttf.h>
#include <sdl1/FileOps.hpp>
#endif

#ifdef USE_SDL2
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_ttf.h>
#include <sdl2/FileOps.hpp>
#endif

#include <common/types.hpp>
#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <common/Font.hpp>
#include <log/Log.hpp>

using namespace fs;

class Glyph {
public:
    SDL_Surface* src = nullptr;
    U32 width, height;
    S32 bearingX, bearingY;
    S32 advance;

    void blitTo(S32& offsetX, S32& offsetY, const Color& color, Surface& target) {
        auto data = (const U8*) src->pixels;
        for (U32 y = 0; y < height; ++y) {
            for (U32 x = 0; x < width; ++x) {
                auto alpha = data[y * src->pitch + x];
                auto m = alpha / 255.0f;
                target.setPixel(x + offsetX + bearingX,
                                y + offsetY - bearingY,
                                Color(color.r * m, color.g * m, color.b * m, alpha));
            }
        }
        offsetX += advance;
    }

    ~Glyph() {
        if (src)
            SDL_FreeSurface(src);
    }
};

class ParsedFont {
    TTF_Font* font;
    U32 size;

public:

    // ParsedFont(const String& path, U32 size) : size{size} {
    //     font = TTF_OpenFont(path.c_str(), size);

    ParsedFont(FileOps& fops, U32 size) : size{size} {
        font = TTF_OpenFontRW(&fops, 0, size);
        if (!font) {
            logE("Error parsing font: ", TTF_GetError());
        } else
            logI("Success [font]");
    }


    U32 getGlyphIndex(const String& text, U32& offset) {
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

    HashMap<U32, std::shared_ptr<Glyph>> cache;

    std::shared_ptr<Glyph> getGlyph(const String& text, U32& i) {
        if (!font)
            return nullptr;

        auto glyph = getGlyphIndex(text, i);
        auto it = cache.find(glyph);
        if (it != cache.end())
            return it->second;

        int advance, maxy, miny, maxx, minx;
        if (TTF_GlyphMetrics(font, glyph, &minx, &maxx, &miny, &maxy, &advance) != 0)
            return nullptr;

        auto surface = TTF_RenderGlyph_Shaded(font, glyph, SDL_Color{255, 255, 255, 255}, SDL_Color{0, 0, 0, 0});
        if (!surface)
            return nullptr;

        auto inst = std::make_shared<Glyph>();
        inst->src = nullptr; // surface;
        inst->advance = advance;
        inst->width = maxx - minx;
        inst->height = maxy - miny;
        inst->bearingX = minx;
        inst->bearingY = miny;

        cache[glyph] = inst;
        return inst;
    }

    ~ParsedFont() {
        if (font)
            TTF_CloseFont(font);
    }
};

class TTFFont : public Font {
public:
    static bool init() {
        static bool wasInit = false;
        static bool success = false;
        if (!wasInit) {
            success = TTF_Init() == 0;
            if (!success)
                logE("Could not init SDL_ttf");
            wasInit = true;
        }
        return success;
    }

    U32 currentSize = ~U32{};
    std::shared_ptr<ParsedFont> currentFont;
    HashMap<U32, std::shared_ptr<ParsedFont>> fontCache;

    // String fops;
    // TTFFont(std::shared_ptr<File>& file) : fops{file->getUID()} {
    //     init();
    // }

    FileOps fops;
    TTFFont(std::shared_ptr<File>& file) : fops{file} {
        init();
    }

    void setSize(U32 size) {
        auto it = fontCache.find(size);
        if (it == fontCache.end()) {
            currentFont = std::make_shared<ParsedFont>(fops, size);
            fontCache[size] = currentFont;
        } else {
            currentFont = it->second;
        }
        currentSize = size;
    }

    virtual std::shared_ptr<Surface> print(U32 size, const Color& color, const String& text, const Rect& padding, Vector<S32>& advance) {
        advance.clear();
        if (text.empty())
            return nullptr;
        auto surface = std::make_shared<Surface>();
        setSize(size);
        U32 width = 0;
        U32 height = 0;
        Vector<std::shared_ptr<Glyph>> glyphs;
        for (U32 i = 0, len = text.size(); i < len; ++i) {
            auto glyph = currentFont->getGlyph(text, i);
            if (!glyph)
                continue;

            glyphs.push_back(glyph);
            advance.push_back(glyph->advance);
            width += glyph->advance;
            height = std::max(height, size + (glyph->height - glyph->bearingY));
        }

        if (!advance.empty()) {
            advance[0] += padding.x;
        }

        if (height && width) {
            surface->resize(width + padding.x + padding.width, height + padding.y + padding.height);
            S32 x = padding.x, y = size + padding.y;
            for (auto& glyph : glyphs)
                glyph->blitTo(x, y, color, *surface);
        } else {
            surface->resize(10, 10);
            auto data = surface->data();
            Color c{"rgba{128,0,0,255}"};
            for (U32 i=0; i<10*10; ++i)
                data[i] = c.toU32();
        }

        return surface;
    }
};

class FontParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        if (!TTFFont::init())
            return false;
        return std::static_pointer_cast<Font>(std::make_shared<TTFFont>(file));
    }
};

static Parser::Shared<FontParser> font{"ttf"};

#endif
