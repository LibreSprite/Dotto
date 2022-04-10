// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <SDL/SDL.h>

#include <SDL/SDL_video.h>
#include <common/match.hpp>
#include <common/Rect.hpp>
#include <common/Surface.hpp>
#include <cstdint>
#include <gui/Graphics.hpp>
#include <gui/Texture.hpp>
#include <log/Log.hpp>

extern int SDL_SoftStretchAlpha(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, SDL_Color *multiply);
extern int SDL_BlitSurfaceMul(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, SDL_Color *multiply);
extern void SDL_FillRectAlpha(SDL_Surface *src, SDL_Rect *rect, SDL_Color *color, Uint8 a);

class SDLTextureInfo : public Texture, public TextureInfo {
public:
    SDL_Surface* surface = nullptr;
    U32 width = 0;
    U32 height = 0;
    F32 iwidth = 0;
    F32 iheight = 0;
    Rect dirtyRegion{0, 0, ~U32{}, ~U32{}};

    void setDirty(const Rect& region) override {
        dirtyRegion.expand(region);
    }

    ~SDLTextureInfo() {
        if (surface)
            SDL_FreeSurface(surface);
    }
};

class SDLGraphics : public Graphics, public std::enable_shared_from_this<SDLGraphics> {
public:
    F32 iwidth, iheight;
    S32 width, height;

    SDL_Surface* screen;
    std::shared_ptr<SDLTextureInfo> activeTexture;

    SDLGraphics(SDL_Surface* screen) : screen{screen} {}

    void begin(Rect& globalRect, Color& clearColor) {
        clip = globalRect;
        width = globalRect.width;
        iwidth = 2.0f / width;
        height = globalRect.height;
        iheight = 2.0f / height;

        SDL_FillRect(screen, nullptr, SDL_MapRGBA(screen->format, clearColor.r, clearColor.g, clearColor.b, clearColor.a));
    }

    void upload(Surface& surface, SDLTextureInfo* texture) {
        texture->dirtyRegion = Rect{};

        if (texture->surface) {
            if (texture->surface->w != surface.width() ||
                texture->surface->h != surface.height()) {
                SDL_FreeSurface(texture->surface);
                texture->surface = nullptr;
            }
        }

        if (!texture->surface) {
            texture->surface = SDL_CreateRGBSurfaceFrom(surface.data(),
                                                        surface.width(),
                                                        surface.height(),
                                                        32,
                                                        surface.width() * sizeof(Surface::PixelType),
                                                        0xFF << Color::Rshift,
                                                        0xFF << Color::Gshift,
                                                        0xFF << Color::Bshift,
                                                        0xFF << Color::Ashift);
        } else {
            auto out = (Surface::PixelType*) texture->surface->pixels;
            auto in = surface.data();
            for (U32 i=0, size = surface.width() * surface.height(); i < size; ++i)
                out[i] = in[i];
        }

        texture->width = surface.width();
        texture->iwidth = 1.0f / texture->width;
        texture->height = surface.height();
        texture->iheight = 1.0f / texture->height;
    }

    struct Rectf {
        F32 x, y, w, h;
        F32 u0, v0, u1, v1;
        U8 r, g, b, a;
        bool flip;
    };

    bool debug = false;

    void push(F32 z, const Rectf& rect) {
        F32 x1 = rect.x,
            y1 = rect.y,
            x2 = rect.x + rect.w,
            y2 = rect.y + rect.h,
            u0 = rect.u0,
            v0 = rect.v0,
            u1 = rect.u1,
            v1 = rect.v1;

        if (x1 >= clip.right() ||
            x2 <= clip.x ||
            y1 >= clip.bottom() ||
            y2 <= clip.y) {
            if (debug)
                logI("Texture clipped away");
            return;
        }

        if (x1 < clip.x) {
            if (rect.w) {
                u0 += (clip.x - x1) / rect.w;
            }
            x1 = clip.x;
        }
        if (x2 > clip.right()) {
            if (rect.w) {
                u1 -= (x2 - clip.right()) / rect.w;
            }
            x2 = clip.right();
        }

        if (y1 < clip.y) {
            if (rect.h) {
                v0 += (clip.y - y1) / rect.h;
            }
            y1 = clip.y;
        }
        if (y2 > clip.bottom()) {
            if (rect.h) {
                v1 -= (y2 - clip.bottom()) / rect.h;
            }
            y2 = clip.bottom();
        }

        if (debug)
            logI("Pushing ", x1, " ", y1, " => ", x2, " ", y2);

        // push({x1, y1, z, u0, v0, rect.r, rect.g, rect.b, rect.a, rect.flip});
        // push({x1, y2, z, u0, v1, rect.r, rect.g, rect.b, rect.a, rect.flip});
        // push({x2, y1, z, u1, v0, rect.r, rect.g, rect.b, rect.a, rect.flip});
        // push({x1, y2, z, u0, v1, rect.r, rect.g, rect.b, rect.a, rect.flip});
        // push({x2, y2, z, u1, v1, rect.r, rect.g, rect.b, rect.a, rect.flip});
        // push({x2, y1, z, u1, v0, rect.r, rect.g, rect.b, rect.a, rect.flip});

        SDL_Rect dest;
        dest.x = x1;
        dest.y = y1;
        dest.w = x2 - x1;
        dest.h = y2 - y1;
        if (activeTexture) {
            SDL_Rect src;
            src.x = u0 * activeTexture->width;
            src.y = v0 * activeTexture->height;
            src.w = (u1 - u0) * activeTexture->width;
            src.h = (v1 - v0) * activeTexture->height;

            if (src.h && src.w) {
                SDL_Color mul{
                    rect.r,
                    rect.g,
                    rect.b,
                    rect.a
                };

                SDL_Color* mulp = (rect.r != 0xFF || rect.g != 0xFF || rect.b != 0xFF || rect.a != 0xFF) ? &mul : nullptr;

                if (dest.w != src.w || dest.h != src.h) {
                    SDL_SoftStretchAlpha(activeTexture->surface, &src, screen, &dest, mulp);
                } else if (mulp) {
                    SDL_BlitSurfaceMul(activeTexture->surface, &src, screen, &dest, mulp);
                } else {
                    SDL_BlitSurface(activeTexture->surface, &src, screen, &dest);
                }
            }

        } else {
            if (rect.a != 0xFF) {
                SDL_Color color{rect.r, rect.g, rect.b};
                SDL_FillRectAlpha(screen, &dest, &color, rect.a);
            } else {
                SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, rect.r, rect.g, rect.b));
            }
        }
    }

    void push(std::shared_ptr<SDLTextureInfo>& texture, const BlitSettings& settings) {
        debug = settings.debug;
        F32 x = settings.destination.x;
        F32 y = settings.destination.y;
        F32 z = settings.zIndex;
        F32 w = settings.destination.width;
        F32 h = settings.destination.height;
        U8 r = settings.multiply.r;
        U8 g = settings.multiply.g;
        U8 b = settings.multiply.b;
        U8 a = settings.multiply.a * alpha;

        if (a <= 0)
            return;

        if (!clip.overlaps(settings.destination)) {
            if (debug)
                logI("Killed by the clip");
            return;
        }

        activeTexture = texture;

        F32 sW = settings.nineSlice.width;
        F32 sH = settings.nineSlice.height;
        S32 textureWidth = texture ? texture->width : 1;
        S32 textureHeight = texture ? texture->height : 1;
        F32 textureIwidth = texture ? texture->iwidth : 1;
        F32 textureIheight = texture ? texture->iheight : 1;

        if (sW == 0 && settings.nineSlice.x != 0)
            sW = textureWidth - settings.nineSlice.x * 2;
        if (sH == 0 && settings.nineSlice.y != 0)
            sH = textureHeight - settings.nineSlice.y * 2;

        if (sW <= 0 || sH <= 0) {
            push(z, {
                    x, y,
                    w, h,
                    settings.source.x / F32(textureWidth), settings.source.y / F32(textureHeight),
                    settings.source.right() / F32(textureWidth), settings.source.bottom() / F32(textureHeight),
                    r, g, b, a,
                    settings.flip
                });
        } else {
            F32 sX = settings.nineSlice.x;
            F32 sY = settings.nineSlice.y;
            F32 nsX = sX * textureIwidth;
            F32 nsY = sY * textureIheight;
            F32 nsW = sW * textureIwidth;
            F32 nsH = sH * textureIheight;
            F32 rW = textureWidth - sW - sX;
            F32 rH = textureHeight - sH - sY;

            push(z, {x,          y, sX,          sY, 0.0f,      0.0f, nsX,       nsY, r, g, b, a});
            push(z, {x + sX,     y, w - sX - rW, sY, nsX,       0.0f, nsX + nsW, nsY, r, g, b, a});
            push(z, {x + w - rW, y, rW,          sY, nsX + nsW, 0.0f, 1.0f,      nsY, r, g, b, a});

            y += sY;
            push(z, {x,        y, sX,      h-sY-rH, 0.0f,      nsY, nsX,       nsY + nsH, r, g, b, a});
            push(z, {x + sX,   y, w-sX-rW, h-sY-rH, nsX,       nsY, nsX + nsW, nsY + nsH, r, g, b, a});
            push(z, {x + w-rW, y, rW,      h-sY-rH, nsX + nsW, nsY, 1.0f,      nsY + nsH, r, g, b, a});

            y += h - sY - rH;
            push(z, {x,          y, sX,          rH, 0.0f,      nsY + nsH, nsX, 1.0f, r, g, b, a});
            push(z, {x + sX,     y, w - sX - rW, rH, nsX,       nsY + nsH, nsX + nsW, 1.0f, r, g, b, a});
            push(z, {x + w - rW, y, rW,          rH, nsX + nsW, nsY + nsH, 1.0f, 1.0f, r, g, b, a});
        }
    }

    Vector<fork_ptr<Texture>> textures;

    std::shared_ptr<SDLTextureInfo> getTexture(Surface& surface) {
        auto texture = surface.info().get<SDLTextureInfo>(this);

        if (!texture) {
            texture = std::make_shared<SDLTextureInfo>();
            fork_ptr<Texture> ptr{std::static_pointer_cast<Texture>(texture)};
            textures.push_back(ptr);
            surface.info().set(this, std::move(ptr));
        }

        return texture;
    }

    void blit(const BlitSettings& settings) override {
        std::shared_ptr<SDLTextureInfo> texture;
        if (settings.surface) {
            auto& surface = *settings.surface;
            texture = getTexture(surface);

            if (!texture->dirtyRegion.empty()) {
                if (settings.debug)
                    logI("Uploading surface");
                upload(surface, texture.get());
            }
        } else if (settings.multiply.a == 0) {
            return; // no texture + no color = no op
        }

        push(texture, settings);
    }

    Rect pushClipRect(const Rect& rect) override {
        auto copy = clip;
        clip.intersect(rect);
        return copy;
    }

    void setClipRect(const Rect& rect) override {
        clip = rect;
    }

    Surface* read() override {
        return nullptr;
    }

    void write() override {
    }
};

