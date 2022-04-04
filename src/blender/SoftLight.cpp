// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>

class SoftLight : public Blender {
public:
    Surface::PixelType blendPixel(const Color& a, const Color& b, F32 alpha) override {
        alpha *= b.a * (1.0f / 255.0f);

        U32 R = ((a.r * b.r) >> 8);
        U32 G = ((a.g * b.g) >> 8);
        U32 B = ((a.b * b.b) >> 8);

        color.r = R + (a.r * (255 - ((255 - a.r) * (255 - b.r) >> 8) - R) >> 8);
        color.g = G + (a.g * (255 - ((255 - a.g) * (255 - b.g) >> 8) - G) >> 8);
        color.b = B + (a.b * (255 - ((255 - a.b) * (255 - b.b) >> 8) - B) >> 8);

        color.r = a.r * (1 - alpha) + color.r * alpha + 0.5f;
        color.g = a.g * (1 - alpha) + color.g * alpha + 0.5f;
        color.b = a.b * (1 - alpha) + color.b * alpha + 0.5f;

        color.a = a.a + alpha * (255 - a.a) + 0.5f;
        return color.toU32();
    }
};

static Blender::Shared<SoftLight> reg{"softlight"};
