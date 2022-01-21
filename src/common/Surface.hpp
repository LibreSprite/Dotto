// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <memory>
#include <variant>

#include <common/Color.hpp>
#include <common/types.hpp>
#include <gui/Texture.hpp>

class Surface : public std::enable_shared_from_this<Surface> {
public:
    using PixelType = U32;

    U32 width() const {return _width;}
    U32 height() const {return _height;}

    void resize(U32 width, U32 height) {
        _width = width;
        _height = height;
        pixels.resize(width * height);
    }

    PixelType* data() {return pixels.data();}

    U32 dataSize() {return _width * _height * sizeof(PixelType);};

    void setDirty() {
        if (textureInfo)
            textureInfo->setDirty();
    }

    const Vector<PixelType>& getPixels() {
        return pixels;
    }

    Color getPixel(U32 x, U32 y) {
        U32 index = x + y * _width;
        return (index >= _width * _height) ? Color{} : getColor(pixels[index]);
    }

    void setPixelUnsafe(U32 x, U32 y, PixelType pixel) {
        U32 index = x + y * _width;
        pixels[index] = pixel;
        setDirty();
    }

    void setPixel(U32 x, U32 y, PixelType pixel) {
        U32 index = x + y * _width;
        if (index < _width * _height) {
            setDirty();
            pixels[index] = pixel;
        }
    }

    void setPixel(U32 x, U32 y, const Color& color) {
        setPixel(x, y, color.toU32());
    }

    Color getColor(PixelType pixel) {
        return pixel;
    }

    std::shared_ptr<TextureInfo> textureInfo;

private:
    U32 _width = 0, _height = 0;
    Vector<PixelType> pixels;
};
