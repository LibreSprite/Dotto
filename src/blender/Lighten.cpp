// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>

class Lighten : public Blender {
public:
    Surface::PixelType blendPixel(const Color& a, const Color& b, F32 alpha) override {
        alpha *= b.a * (1.0f / 255.0f);
        if (U32(a.r) + U32(a.g) + U32(a.b) < U32(b.r) + U32(b.g) + U32(b.b)) {
            color.r = a.r * (1 - alpha) + b.r * alpha + 0.5f;
            color.g = a.g * (1 - alpha) + b.g * alpha + 0.5f;
            color.b = a.b * (1 - alpha) + b.b * alpha + 0.5f;
            color.a = a.a + alpha * (255 - a.a) + 0.5f;
            return color.toU32();
        } else {
            return a.toU32();
        }
    }
};

static Blender::Shared<Lighten> reg{"lighten"};
