// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>

class HardLight : public Blender {
public:
    Surface::PixelType blendPixel(const Color& a, const Color& b, F32 alpha) override {
        alpha *= b.a * (1.0f / 255.0f);
        if (a.r < 128) {
            color.r = a.r * (1 - alpha) + ((a.r * b.r) >> 7) * alpha + 0.5f;
        } else {
            color.r = a.r * (1 - alpha) + (255 - (((255 - a.r) * (255 - b.r)) >> 7)) * alpha + 0.5f;
        }

        if (a.g < 128) {
            color.g = a.g * (1 - alpha) + ((a.g * b.g) >> 7) * alpha + 0.5f;
        } else {
            color.g = a.g * (1 - alpha) + (255 - (((255 - a.g) * (255 - b.g)) >> 7)) * alpha + 0.5f;
        }

        if (a.b < 128) {
            color.b = a.b * (1 - alpha) + ((a.b * b.b) >> 7) * alpha + 0.5f;
        } else {
            color.b = a.b * (1 - alpha) + (255 - (((255 - a.b) * (255 - b.b)) >> 7)) * alpha + 0.5f;
        }
        color.a = a.a + alpha * (255 - a.a) + 0.5f;
        return color.toU32();
    }
};

static Blender::Shared<HardLight> reg{"hardlight"};
