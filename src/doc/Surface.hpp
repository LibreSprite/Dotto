// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>
#include <doc/Palette.hpp>

template<typename _PixelType>
class Surface {
    using PixelType = _PixelType;

public:
    U32 width() const {return _width;}
    U32 height() const {return _height;}
    void resize(U32 width, U32 height) {pixels.resize(width * height);}
    PixelType* data() {return pixels.data();}

    Color getPixel(U32 x, U32 y) {
        U32 index = x + y * _width;
        return (index >= _width * _height) ? Color{} : getColor(pixels[index]);
    }

    Color getColor(PixelType pixel) {
        if constexpr (std::is_same_v<PixelType, U32>) {
            return pixel;
        } else {
            if (pixel < palette.size())
                return palette[pixel];
            return {pixel, pixel, pixel};
        }
    }

    Palette palette;

private:
    U32 _width = 0, _height = 0;
    Vector<_PixelType> pixels;
};

using SurfaceRGBA = Surface<U32>;
using Surface256 = Surface<U8>;
