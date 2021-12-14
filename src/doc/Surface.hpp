// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <variant>

#include <common/types.hpp>
#include <doc/Palette.hpp>

class Texture;

template<typename _PixelType>
class GenericSurface {
    using PixelType = _PixelType;

public:
    U32 width() const {return _width;}
    U32 height() const {return _height;}

    void resize(U32 width, U32 height) {
        _width = width;
        _height = height;
        pixels.resize(width * height);
    }

    PixelType* data() {return pixels.data();}

    Color getPixel(U32 x, U32 y) {
        U32 index = x + y * _width;
        return (index >= _width * _height) ? Color{} : getColor(pixels[index]);
    }

    void setPixelUnsafe(U32 x, U32 y, PixelType pixel) {
        U32 index = x + y * _width;
        pixels[index] = pixel;
    }

    void setPixel(U32 x, U32 y, PixelType pixel) {
        U32 index = x + y * _width;
        if (index < _width * _height)
            pixels[index] = pixel;
    }

    void setPixel(U32 x, U32 y, const Color& color) {
        U32 index = x + y * _width;
        if (index < _width * _height) {
            if constexpr (std::is_same_v<PixelType, U32>) {
                pixels[index] = color.toU32();
            } else {
                pixels[index] = findClosestColorIndex(palette, color);
            }
        }
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
    std::shared_ptr<Texture> _texture;
    bool dirty;
};

using SurfaceRGBA = GenericSurface<U32>;
using Surface256 = GenericSurface<U8>;
using Surface = std::variant<Surface256, SurfaceRGBA>;
