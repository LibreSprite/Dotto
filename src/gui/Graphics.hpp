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
    struct BlitSettings {
        std::shared_ptr<Surface> surface;
        const Rect& source;
        const Rect& destination;
        const Rect& nineSlice;
        S32 zIndex;
    };

    virtual void blit(const BlitSettings& settings){}
    virtual Rect pushClipRect(const Rect& rect){return clip;}
    virtual void setClipRect(const Rect& rect){}

    bool isEmptyClipRect() {
        return clip.empty();
    }
};
