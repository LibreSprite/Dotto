// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>

class Multiply : public Blender {
public:
    Surface::PixelType blendPixel(const Color& a, const Color& b, F32 alpha) override {
        alpha *= b.a * (1.0f / 255.0f);
        color.r = a.r * (1 - alpha) + ((a.r * b.r) >> 8) * alpha + 0.5f;
        color.g = a.g * (1 - alpha) + ((a.g * b.g) >> 8) * alpha + 0.5f;
        color.b = a.b * (1 - alpha) + ((a.b * b.b) >> 8) * alpha + 0.5f;
        color.a = a.a + alpha * (255 - a.a) + 0.5f;
        return color.toU32();
    }
};

static Blender::Shared<Multiply> reg{"multiply"};
