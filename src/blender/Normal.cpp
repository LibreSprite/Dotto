// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>

class Normal : public SoftBlender {
public:
    Surface::PixelType blendPixel(const Color& a, const Color& b, F32 alpha) override {
        alpha *= b.a * (1.0f / 255.0f);
        color.r = a.r * (1 - alpha) + b.r * alpha;
        color.g = a.g * (1 - alpha) + b.g * alpha;
        color.b = a.b * (1 - alpha) + b.b * alpha;
        color.a = a.a + alpha * (255 - a.a);
        return color.toU32();
    }
};

static Blender::Shared<Normal> reg{"normal"};
