// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Surface.hpp>

std::shared_ptr<Surface> Surface::clone() {
    auto other = std::make_shared<Surface>();
    other->resize(_width, _height);
    other->setPixels(getPixels());
    return other;
}

TextureInfo& Surface::info() {
    if (!_textureInfo)
        _textureInfo = std::make_unique<TextureInfo>();
    return *_textureInfo;
}

void Surface::resize(U32 width, U32 height) {
    _width = width;
    _height = height;
    pixels.resize(width * height);
}

void Surface::setDirty(const Rect& region) {
    if (_textureInfo)
        _textureInfo->setDirty(region);
}

void Surface::setPixels(const Vector<PixelType>& read) {
    if (read.size() != pixels.size())
        return;
    pixels = read;
    setDirty({0, 0, _width, _height});
}

void Surface::setHLine(S32 x, S32 y, S32 w, PixelType pixel) {
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (S32(x + w) > S32(_width)) {
        w = _width - x;
    }
    if (w <= 0 || U32(y) >= _height) {
        return;
    }
    U32 index = y * _width;
    for (S32 e = x + w; x != e; ++x) {
        pixels[index + x] = pixel;
    }
}

void Surface::antsHLine(S32 x, S32 y, S32 w, U32 age, PixelType A, PixelType B) {
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (S32(x + w) > S32(_width)) {
        w = _width - x;
    }
    if (w <= 0 || U32(y) >= _height) {
        return;
    }
    U32 index = y * _width;
    age += y;
    for (S32 e = x + w; x != e; ++x) {
        pixels[index + x] = -((age + x) & 4) ? A : B;
    }
}

void Surface::setVLine(S32 x, S32 y, S32 h, PixelType pixel) {
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (S32(y + h) > S32(_height)) {
        h = _height - y;
    }
    if (h <= 0 || U32(x) >= _width) {
        return;
    }
    U32 index = y * _width + x;
    for (; h; --h) {
        pixels[index] = pixel;
        index += _width;
    }
}

void Surface::antsVLine(S32 x, S32 y, S32 h, U32 age, PixelType A, PixelType B) {
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (S32(y + h) > S32(_height)) {
        h = _height - y;
    }
    if (h <= 0 || U32(x) >= _width) {
        return;
    }
    U32 index = y * _width + x;
    age += x;
    for (; h; --h) {
        pixels[index] = ((age + y++) & 4) ? A : B;
        index += _width;
    }
}

void Surface::fillRect(const Rect& rect, PixelType pixel) {
    S32 x = rect.x;
    S32 y = rect.y;
    S32 w = rect.width;
    S32 h = rect.height;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (S32(x + w) > S32(_width)) {
        w = _width - x;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (S32(y + h) > S32(_height)) {
        h = _height - y;
    }
    if (w <= 0 || U32(y) >= _height || h <= 0 || U32(x) >= _width) {
        return;
    }
    U32 index = y * _width + x;
    for (; h; --h) {
        for (S32 e = 0; e < w; ++e) {
            pixels[index + e] = pixel;
        }
        index += _width;
    }
}

void Surface::setPixel(U32 x, U32 y, PixelType pixel) {
    U32 index = x + y * _width;
    if (index < _width * _height) {
        setDirty({S32(x), S32(y), 1, 1});
        pixels[index] = pixel;
    }
}
