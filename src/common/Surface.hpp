// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <memory>
#include <variant>

#include <common/Color.hpp>
#include <common/Rect.hpp>
#include <common/types.hpp>
#include <gui/Texture.hpp>

class Surface : public std::enable_shared_from_this<Surface> {
public:
    using PixelType = U32;

private:
    U32 _width = 0, _height = 0;
    Vector<PixelType> pixels;
    std::unique_ptr<TextureInfo> _textureInfo;

public:
    U32 width() const {return _width;}
    U32 height() const {return _height;}
    Rect rect() const {return {0, 0, _width, _height};}
    PixelType* data() {return pixels.data();}
    U32 dataSize() {return _width * _height * sizeof(PixelType);};
    const Vector<PixelType>& getPixels() {return pixels;}

    std::shared_ptr<Surface> clone();
    TextureInfo& info();
    void resize(U32 width, U32 height);
    void setDirty(const Rect& region);
    void setPixels(const Vector<PixelType>& read);

    Color getPixel(U32 x, U32 y) {
        U32 index = x + y * _width;
        return (index >= _width * _height) ? Color{} : getColor(pixels[index]);
    }

    PixelType getPixelUnsafe(U32 x, U32 y) {
        return pixels[x + y * _width];
    }

    void setPixelUnsafe(U32 x, U32 y, PixelType pixel) {
        U32 index = x + y * _width;
        pixels[index] = pixel;
        setDirty({S32(x), S32(y), 1, 1});
    }

    void setPixel(U32 x, U32 y, PixelType pixel);

    void setPixel(U32 x, U32 y, const Color& color) {
        setPixel(x, y, color.toU32());
    }

    Color getColor(PixelType pixel) {
        return pixel;
    }

    Surface& operator = (const Surface& other) {
        _width = other._width;
        _height = other._height;
        pixels = other.pixels;
        setDirty(rect());
        return *this;
    }
};
