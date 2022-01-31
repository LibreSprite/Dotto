// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/Rect.hpp>
#include <common/Surface.hpp>

class Graphics {
protected:
    Rect clip;

public:
    F32 scale = 1;
    F32 alpha = 1.0f;

    struct BlitSettings {
        std::shared_ptr<Surface> surface;
        const Rect& source;
        const Rect& destination;
        const Rect& nineSlice;
        S32 zIndex;
        Color multiply;
        bool debug;
        bool flip;
    };

    virtual void blit(const BlitSettings& settings){}
    virtual Rect pushClipRect(const Rect& rect){return clip;}
    virtual void setClipRect(const Rect& rect){}
    virtual Surface* read() {return nullptr;}
    virtual void write() {}

    bool isEmptyClipRect() {
        return clip.empty();
    }
};
