// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <variant>

#include <common/types.hpp>
#include <doc/Palette.hpp>

class Texture;

class Surface {
    using PixelType = U32;

public:
    U32 width() const {return _width;}
    U32 height() const {return _height;}

    void resize(U32 width, U32 height) {
        _width = width;
        _height = height;
        pixels.resize(width * height);
    }

    PixelType* data() {return pixels.data();}

    U32 dataSize() {return pixels.size() * sizeof(PixelType);};

    bool isDirty() const {return dirty;}
    void setDirty() {dirty = true;}
    void clearDirty() {dirty = false;}

    Color getPixel(U32 x, U32 y) {
        U32 index = x + y * _width;
        return (index >= _width * _height) ? Color{} : getColor(pixels[index]);
    }

    void setPixelUnsafe(U32 x, U32 y, PixelType pixel) {
        U32 index = x + y * _width;
        pixels[index] = pixel;
        dirty = true;
    }

    void setPixel(U32 x, U32 y, PixelType pixel) {
        U32 index = x + y * _width;
        if (index < _width * _height) {
            pixels[index] = pixel;
            dirty = true;
        }
    }

    void setPixel(U32 x, U32 y, const Color& color) {
        U32 index = x + y * _width;
        if (index < _width * _height) {
            dirty = true;
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
            return Color(pixel, pixel, pixel);
        }
    }

    Palette palette;
    std::shared_ptr<Texture> texture;

private:
    U32 _width = 0, _height = 0;
    Vector<PixelType> pixels;
    bool dirty;
};
