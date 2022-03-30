// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Surface.hpp>

class Palette : public Injectable<Palette>, public std::enable_shared_from_this<Palette> {
protected:
    Vector<Color> colors;

public:
    std::size_t size() {return colors.size();}

    void push(const Color& color) {colors.push_back(color);}

    Color* at(std::size_t index) {
        return (index >= colors.size()) ? nullptr : &colors[index];
    }

    Palette& operator = (const Palette& other) {
        colors = other.colors;
        return *this;
    }

    virtual void loadFromSurface(Surface& surface, U32 maxColors) = 0;

    virtual U32 findClosestColorIndex(const Color& color) = 0;

    Color findClosestColor(const Color& color) {
        if (colors.empty())
            return {color.r, color.g, color.b, color.a};
        return colors[findClosestColorIndex(color)];
    }

};
