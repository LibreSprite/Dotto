// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <tools/Pencil.hpp>

class Surface;

class Eraser : public Pencil {
public:
    Eraser() {
        preview.draw = Preview::drawFilledSolid;
        preview.overlayColor = 0x3FFFFFFF;
        preview.altColor = 0x3F000000;
    }

    Preview* getPreview() override {
        return &preview;
    }

    void initPaint(Surface* surface) override {
        paint = inject<Command>{"paint"};
        paint->load({
                {"selection", selection},
                {"preview", true},
                {"surface", surface->shared_from_this()},
                {"mode", which == 1 ? "erase" : "normal"},
            });
    }

};

static Tool::Shared<Eraser> erase{"eraser"};
