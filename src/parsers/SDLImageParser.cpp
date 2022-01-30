// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <SDL2/SDL_image.h>

#include <common/Color.hpp>
#include <common/Parser.hpp>
#include <common/Surface.hpp>
#include <log/Log.hpp>

class SDLImage {
public:
    SDLImage() {
        IMG_Init(IMG_INIT_JPG|IMG_INIT_TIF|IMG_INIT_WEBP);
    }

    ~SDLImage() {
        IMG_Quit();
    }

    static SDLImage& instance() {
        static SDLImage image;
        return image;
    }

    std::shared_ptr<Surface> load(Vector<U8>& data) {
        auto ops = SDL_RWFromMem(data.data(), data.size());
        SDL_Surface* sdl = IMG_Load_RW(ops, 1);
        if (!sdl)
            return nullptr;
        auto surface = std::make_shared<Surface>();
        surface->resize(sdl->w, sdl->h);
        surface->setDirty();
        SDL_LockSurface(sdl);

        Color color;
        U32 bpp = sdl->format->BytesPerPixel;
        for (U32 y = 0; y < sdl->h; ++y) {
            for (U32 x = 0; x < sdl->w; ++x) {
                auto ptr = static_cast<U8*>(sdl->pixels) + y * sdl->pitch + x * bpp;
                U32 pixel = 0;
                switch (bpp) {
                case 1: pixel = ptr[0]; break;
                case 2: pixel = reinterpret_cast<U16*>(ptr)[0]; break;
                case 3: pixel = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16); break;
                case 4: pixel = reinterpret_cast<U32*>(ptr)[0]; break;
                }
                SDL_GetRGBA(pixel, sdl->format, &color.r, &color.g, &color.b, &color.a);
                surface->data()[y * sdl->w + x] = color.toU32();
            }
        }

        SDL_UnlockSurface(sdl);
        SDL_FreeSurface(sdl);
        return surface;
    }
};

class SDLImageParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        U32 size = file->size();
        Vector<U8> data;
        data.resize(size);
        file->read(data.data(), size);
        return SDLImage::instance().load(data);
    }
};

static Parser::Shared<SDLImageParser> bmp{"bmp", {"image"}};
static Parser::Shared<SDLImageParser> gif{"gif", {"image"}};
static Parser::Shared<SDLImageParser> jpg{"jpg", {"image"}};
static Parser::Shared<SDLImageParser> jpeg{"jpeg", {"image"}};
static Parser::Shared<SDLImageParser> lbm{"lbm", {"image"}};
static Parser::Shared<SDLImageParser> pcx{"pcx", {"image"}};
// static Parser::Shared<SDLImageParser> png{"png", {"image"}};
static Parser::Shared<SDLImageParser> pnm{"pnm", {"image"}};
static Parser::Shared<SDLImageParser> svg{"svg", {"image"}};
static Parser::Shared<SDLImageParser> tga{"tga", {"image"}};
static Parser::Shared<SDLImageParser> tiff{"tiff", {"image"}};
static Parser::Shared<SDLImageParser> tif{"tif", {"image"}};
static Parser::Shared<SDLImageParser> webp{"webp", {"image"}};
static Parser::Shared<SDLImageParser> xcf{"xcf", {"image"}};
static Parser::Shared<SDLImageParser> xpm{"xpm", {"image"}};
static Parser::Shared<SDLImageParser> xv{"xv", {"image"}};
