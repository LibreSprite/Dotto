// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#ifdef USE_SDL2

#include <SDL2/SDL_image.h>

#include <common/Color.hpp>
#include <common/Surface.hpp>

#include <sdl2/FileOps.hpp>

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
        surface->setDirty(surface->rect());
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

    bool saveJPG(std::shared_ptr<File> file, std::shared_ptr<Surface> src, int quality = 75) {
#if SDL_IMAGE_MINOR_VERSION > 0 || SDL_IMAGE_PATCHLEVEL >= 2
        if (!src || !file)
            return false;
        SDL_Surface* sdl = SDL_CreateRGBSurfaceFrom(src->data(),
                                                    src->width(),
                                                    src->height(),
                                                    32,
                                                    src->width() * sizeof(Surface::PixelType),
                                                    0xFF << Color::Rshift,
                                                    0xFF << Color::Gshift,
                                                    0xFF << Color::Bshift,
                                                    0xFF << Color::Ashift);
        FileOps fops{file};
        int result = IMG_SaveJPG_RW(sdl, &fops, 0, 75);
        SDL_FreeSurface(sdl);
        return result == 0;
#else
        return false;
#endif
    }
};

#endif
