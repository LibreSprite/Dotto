#ifdef USE_SDL1

#include <SDL.h>

// /*
//     SDL - Simple DirectMedia Layer
//     Copyright (C) 1997-2012 Sam Lantinga
//     This library is free software; you can redistribute it and/or
//     modify it under the terms of the GNU Lesser General Public
//     License as published by the Free Software Foundation; either
//     version 2.1 of the License, or (at your option) any later version.
//     This library is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.
//     You should have received a copy of the GNU Lesser General Public
//     License along with this library; if not, write to the Free Software
//     Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//     Sam Lantinga
//     slouken@libsdl.org
// */
// #include "SDL_config.h"

// /* This a stretch blit implementation based on ideas given to me by
//    Tomasz Cejner - thanks! :)
//    April 27, 2000 - Sam Lantinga
// */


void blend_row4(Uint32 *src, int src_w, Uint32 *dst, int dst_w) {
	int i;
	int pos, inc;
	Uint32 pixel = 0;

        constexpr const Uint32 f = ((1 << 16) | (1 << 8)) / 255;
        Uint8 ar, ag, ab, aa;

	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
            if ( pos >= 0x10000L ) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
                ar = pixel;
                ag = pixel >> 8;
                ab = pixel >> 16;
                aa = pixel >> 24;

                ar = Uint32(ar) * aa * f >> 16;
                ag = Uint32(ag) * aa * f >> 16;
                ab = Uint32(ab) * aa * f >> 16;
            }

            if (aa) {
                Uint32 o = *dst >> 8;
                Uint8 bb = o; o >>= 8;
                Uint8 bg = o; o >>= 8;

                bb = (Uint32(bb) * (255 - aa) * f >> 16) + ab;
                bg = (Uint32(bg) * (255 - aa) * f >> 16) + ag;
                Uint32 br = (o * (255 - aa) * f >> 16) + ar;

                *dst = (Uint32(bb) << 8) | (Uint32(bg) << 16) | (Uint32(br) << 24);
            }

            dst++;
            pos += inc;
	}
}

void blend_row4m(Uint32 *src, int src_w, Uint32 *dst, int dst_w, SDL_Color* multiply) {
	int i;
	int pos, inc;
	Uint32 pixel = 0;

        constexpr const Uint32 f = ((1 << 16) | (1 << 8)) / 255;
        Uint8 ar, ag, ab, aa;

        Uint32 mr = multiply->r * f;
        Uint32 mg = multiply->g * f;
        Uint32 mb = multiply->b * f;
        Uint32 ma = multiply->unused * f;

	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
            if ( pos >= 0x10000L ) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
                ar = pixel;
                ag = pixel >> 8;
                ab = pixel >> 16;
                aa = pixel >> 24;

                ar = Uint32(ar) * aa * f >> 16;
                ag = Uint32(ag) * aa * f >> 16;
                ab = Uint32(ab) * aa * f >> 16;

                ar = Uint32(ar) * mr >> 16;
                ag = Uint32(ag) * mg >> 16;
                ab = Uint32(ab) * mb >> 16;
                aa = aa * ma >> 16;
            }

            if (aa) {
                Uint32 o = *dst >> 8;
                Uint8 bb = o; o >>= 8;
                Uint8 bg = o; o >>= 8;

                bb = (Uint32(bb) * (255 - aa) * f >> 16) + ab;
                bg = (Uint32(bg) * (255 - aa) * f >> 16) + ag;
                Uint32 br = (o * (255 - aa) * f >> 16) + ar;

                *dst = (Uint32(bb) << 8) | (Uint32(bg) << 16) | (Uint32(br) << 24);
            }

            dst++;
            pos += inc;
	}
}

/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/

int SDL_SoftStretchAlpha(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, SDL_Color *multiply) {
    int src_locked;
    int dst_locked;
    int pos, inc;
    int dst_maxrow;
    int src_row, dst_row;
    Uint8 *srcp = NULL;
    Uint8 *dstp;
    SDL_Rect full_src;
    SDL_Rect full_dst;

    const int bpp = dst->format->BytesPerPixel;

    if ( src->format->BitsPerPixel != dst->format->BitsPerPixel ) {
        SDL_SetError("Only works with same format surfaces");
        return(-1);
    }

    /* Verify the blit rectangles */
    if ( srcrect ) {
        if ( (srcrect->x < 0) || (srcrect->y < 0) ||
             ((srcrect->x+srcrect->w) > src->w) ||
             ((srcrect->y+srcrect->h) > src->h) ) {
            SDL_SetError("Invalid source blit rectangle");
            return(-1);
        }
    } else {
        full_src.x = 0;
        full_src.y = 0;
        full_src.w = src->w;
        full_src.h = src->h;
        srcrect = &full_src;
    }
    if ( dstrect ) {
        if ( (dstrect->x < 0) || (dstrect->y < 0) ||
             ((dstrect->x+dstrect->w) > dst->w) ||
             ((dstrect->y+dstrect->h) > dst->h) ) {
            SDL_SetError("Invalid destination blit rectangle");
            return(-1);
        }
    } else {
        full_dst.x = 0;
        full_dst.y = 0;
        full_dst.w = dst->w;
        full_dst.h = dst->h;
        dstrect = &full_dst;
    }

    /* Lock the destination if it's in hardware */
    dst_locked = 0;
    if ( SDL_MUSTLOCK(dst) ) {
        if ( SDL_LockSurface(dst) < 0 ) {
            SDL_SetError("Unable to lock destination surface");
            return(-1);
        }
        dst_locked = 1;
    }
    /* Lock the source if it's in hardware */
    src_locked = 0;
    if ( SDL_MUSTLOCK(src) ) {
        if ( SDL_LockSurface(src) < 0 ) {
            if ( dst_locked ) {
                SDL_UnlockSurface(dst);
            }
            SDL_SetError("Unable to lock source surface");
            return(-1);
        }
        src_locked = 1;
    }

    /* Set up the data... */
    pos = 0x10000;
    inc = (srcrect->h << 16) / dstrect->h;
    src_row = srcrect->y;
    dst_row = dstrect->y;

    /* Perform the stretch blit */
    for ( dst_maxrow = dst_row+dstrect->h; dst_row<dst_maxrow; ++dst_row ) {
        dstp = (Uint8 *)dst->pixels + (dst_row*dst->pitch)
            + (dstrect->x*bpp);
        while ( pos >= 0x10000L ) {
            srcp = (Uint8 *)src->pixels + (src_row*src->pitch)
                + (srcrect->x*bpp);
            ++src_row;
            pos -= 0x10000L;
        }
        if (multiply)
            blend_row4m((Uint32 *)srcp, srcrect->w, (Uint32 *)dstp, dstrect->w, multiply);
        else
            blend_row4((Uint32 *)srcp, srcrect->w, (Uint32 *)dstp, dstrect->w);
        pos += inc;
    }

    /* We need to unlock the surfaces if they're locked */
    if ( dst_locked ) {
        SDL_UnlockSurface(dst);
    }
    if ( src_locked ) {
        SDL_UnlockSurface(src);
    }
    return(0);
}

int SDL_BlitSurfaceMul(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, SDL_Color *multiply) {
    int src_locked;
    int dst_locked;
    SDL_Rect full_src;
    SDL_Rect full_dst;

    full_src.x = 0;
    full_src.y = 0;
    full_src.w = src->w;
    full_src.h = src->h;

    full_dst.x = 0;
    full_dst.y = 0;
    full_dst.w = dst->w;
    full_dst.h = dst->h;

    /* Verify the blit rectangles */
    if (!srcrect) {
        srcrect = &full_src;
    }
    if (!dstrect) {
        dstrect = &full_dst;
    }

    if (dstrect->x < 0) {
        srcrect->x -= dstrect->x;
        srcrect->w += dstrect->x;
        dstrect->x = 0;
    }
    if (dstrect->y < 0) {
        srcrect->y -= dstrect->y;
        srcrect->h += dstrect->y;
        dstrect->y = 0;
    }
    if (dstrect->x + srcrect->w > dst->w) {
        srcrect->w = dst->w - dstrect->x;
    }
    if (dstrect->y + srcrect->h > dst->h) {
        srcrect->h = dst->h - dstrect->y;
    }

    if (srcrect->x <= -int(src->w)) {
        return -1;
    }
    if (srcrect->x < 0) {
        srcrect->w += srcrect->x;
        dstrect->x -= srcrect->x;
        srcrect->x = 0;
    }
    if (srcrect->x >= src->w) {
        return -1;
    }
    if (srcrect->x + srcrect->w > src->w) {
        srcrect->w = src->w - srcrect->x;
    }

    if (srcrect->y <= -int(src->h)) {
        return -1;
    }
    if (srcrect->y < 0) {
        srcrect->h += srcrect->y;
        dstrect->y += -srcrect->y;
        srcrect->y = 0;
    }
    if (srcrect->y >= src->h) {
        return -1;
    }
    if (srcrect->y + srcrect->h > src->h) {
        srcrect->h = src->h - srcrect->y;
    }

    /* Lock the destination if it's in hardware */
    dst_locked = 0;
    if ( SDL_MUSTLOCK(dst) ) {
        if ( SDL_LockSurface(dst) < 0 ) {
            SDL_SetError("Unable to lock destination surface");
            return(-1);
        }
        dst_locked = 1;
    }
    /* Lock the source if it's in hardware */
    src_locked = 0;
    if ( SDL_MUSTLOCK(src) ) {
        if ( SDL_LockSurface(src) < 0 ) {
            if ( dst_locked ) {
                SDL_UnlockSurface(dst);
            }
            SDL_SetError("Unable to lock source surface");
            return(-1);
        }
        src_locked = 1;
    }

    auto out = (Uint8*) dst->pixels;
    auto in = (Uint8*) src->pixels;

    out += dstrect->y * dst->pitch + dstrect->x * 4;
    in += srcrect->y * src->pitch + srcrect->x * 4;

    constexpr const Uint32 f = ((1 << 16) | (1 << 8)) / 255;

    Uint32 mr = multiply->r * f;
    Uint32 mg = multiply->g * f;
    Uint32 mb = multiply->b * f;

    for (int y = 0, my = srcrect->h; y < my; ++y, out += dst->pitch, in += src->pitch) {
        for (int x = 0, mx = srcrect->w; x < mx; ++x) {
            int i = x * 4;
            if (Uint32 a = in[i + 3]) {
                Uint32 a1 = (255 - a) * f;
                a *= f;
                out[i + 1] = (out[i + 1] * a1 + (in[i + 2] * mr >> 16) * a) >> 16;
                out[i + 2] = (out[i + 2] * a1 + (in[i + 1] * mg >> 16) * a) >> 16;
                out[i + 3] = (out[i + 3] * a1 + (in[i + 0] * mb >> 16) * a) >> 16;
            }
        }
    }

    /* We need to unlock the surfaces if they're locked */
    if ( dst_locked ) {
        SDL_UnlockSurface(dst);
    }
    if ( src_locked ) {
        SDL_UnlockSurface(src);
    }
    return 0;
}

void SDL_FillRectAlpha(SDL_Surface *s, SDL_Rect *rect, SDL_Color *color, Uint8 a) {
    SDL_Rect full_s;
    full_s.x = 0;
    full_s.y = 0;
    full_s.w = s->w;
    full_s.h = s->h;
    if (!rect)
        rect = &full_s;

    if (rect->x >= s->w)
        return;
    if (rect->y >= s->h)
        return;

    if (rect->x < 0) {
        rect->w += rect->x;
        rect->x = 0;
    }
    if (rect->y < 0) {
        rect->h += rect->y;
        rect->y = 0;
    }
    if (rect->x + rect->w > s->w)
        rect->w = s->w - rect->x;
    if (rect->y + rect->h > s->h)
        rect->h = s->h - rect->y;

    if (int(rect->w) <= 0)
        return;
    if (int(rect->h) <= 0)
        return;

    /* Lock the source if it's in hardware */
    int s_locked = 0;
    if ( SDL_MUSTLOCK(s) ) {
        if ( SDL_LockSurface(s) < 0 ) {
            SDL_SetError("Unable to lock source surface");
            return;
        }
        s_locked = 1;
    }
    constexpr const Uint32 f = ((1 << 16) | (1 << 8)) / 255;

    Uint32 mr = color->r * (a * f);
    Uint32 mg = color->g * (a * f);
    Uint32 mb = color->b * (a * f);

    Uint32 a1 = (255 - a) * f;
    Uint32 pitch = s->pitch / 4;
    auto pixels = (Uint32*) s->pixels + rect->y * pitch + rect->x;
    for (int y = 0; y < int(rect->h); ++y, pixels += pitch) {
        for (int x = 0; x < int(rect->w); ++x) {
            Uint32 c = pixels[x];
            Uint32 r = ((c >> 24) * a1 + mr) >> 16;
            Uint32 g = (((c >> 16) & 0xFF) * a1 + mg) >> 16;
            Uint32 b = (((c >> 8) & 0xFF) * a1 + mb) >> 16;
            pixels[x] = (b << 8) | (g << 16) | (r << 24);
        }
    }

    if ( s_locked ) {
        SDL_UnlockSurface(s);
    }
}

#endif
