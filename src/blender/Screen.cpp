// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <blender/Blender.hpp>

class Screen : public Blender {
public:
    Surface::PixelType blendPixel(const Color& a, const Color& b, F32 alpha) override {
        alpha *= b.a * (1.0f / 255.0f);
        color.r = std::min<U32>(255, F32(a.r) + b.r * alpha);
        color.g = std::min<U32>(255, F32(a.g) + b.g * alpha);
        color.b = std::min<U32>(255, F32(a.b) + b.b * alpha);
        color.a = a.a + alpha * (255 - a.a);
        return color.toU32();
    }
};

static Blender::Shared<Screen> reg{"screen"};
