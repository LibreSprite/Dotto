// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <tools/Pencil.hpp>

class Surface;

class Eraser : public Pencil {
public:
    void initPaint() override {
        paint = inject<Command>{"paint"};
        if (which == 1) {
            paint->load({
                    {"selection", selection},
                    {"mode", "erase"},
                    {"preview", true}
                });
        } else {
            paint->load({
                    {"selection", selection},
                    {"preview", true}
                });
        }
    }
};

static Tool::Shared<Eraser> erase{"eraser"};
