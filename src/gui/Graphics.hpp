// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <doc/Surface.hpp>
#include <gui/Rect.hpp>

class Graphics {
protected:
    ui::Rect clip;

public:
    struct BlitSettings {
        std::shared_ptr<Surface> surface;
        const ui::Rect& source;
        const ui::Rect& destination;
        const ui::Rect& nineSlice;
        S32 zIndex;
    };

    virtual void blit(const BlitSettings& settings){}
    virtual ui::Rect pushClipRect(const ui::Rect& rect){return clip;}
    virtual void setClipRect(const ui::Rect& rect){}

    bool isEmptyClipRect() {
        return clip.empty();
    }
};
