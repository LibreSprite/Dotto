// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Selection.hpp>
#include <log/Log.hpp>

static Vector<U8> swap;

class SelectionImpl : public Selection {
public:
    Vector<U8> data;
    Rect bounds;

    const Rect& getBounds() const override {return bounds;}
    const Vector<U8>& getData() const override {return data;}

    void expand(const Rect& rect) {
        U32 outSize = rect.width * rect.height;
        swap.resize(outSize);

        for (auto& old : swap)
            old = 0;

        S32 yoffset = bounds.y - rect.y;
        S32 xoffset = bounds.x - rect.x;

        for (U32 y = 0; y < bounds.height; ++y) {
            U32 out = (y + yoffset) * rect.width + xoffset;
            U32 in = y * bounds.width;
            for (U32 x = 0; x < bounds.width; ++x) {
                swap[out + x] = data[in + x];
            }
        }

        std::swap(data, swap);
        bounds = rect;
    }

    void add(const Selection& other) override {
        Rect otherBounds = other.getBounds();
        Rect newBounds = bounds;
        bool grew = newBounds.expand(otherBounds.x, otherBounds.y);
        grew |= newBounds.expand(otherBounds.right(), otherBounds.bottom());
        if (grew)
            expand(newBounds);
        auto& otherData = other.getData();
        for (U32 y = 0; y < otherBounds.height; ++y) {
            for (U32 x = 0; x < otherBounds.width; ++x) {
                U32 amount = otherData[y * otherBounds.width + x];
                if (!amount)
                    continue;
                U32 index = (y + (otherBounds.y - bounds.y)) * bounds.width + x + (otherBounds.x - bounds.x);
                U32 acc = data[index];
                acc += amount;
                data[index] = acc > 0xFF ? 0xFF : acc;
            }
        }
    }

    void blend(const Selection& other) override {
        Rect otherBounds = other.getBounds();
        Rect newBounds = bounds;
        bool grew = newBounds.expand(otherBounds.x, otherBounds.y);
        grew |= newBounds.expand(otherBounds.right(), otherBounds.bottom());
        if (grew)
            expand(newBounds);
        auto& otherData = other.getData();
        for (U32 y = 0; y < otherBounds.height; ++y) {
            for (U32 x = 0; x < otherBounds.width; ++x) {
                U32 amount = otherData[y * otherBounds.width + x];
                if (!amount)
                    continue;
                U32 index = (y + (otherBounds.y - bounds.y)) * bounds.width + x + (otherBounds.x - bounds.x);
                U32 old = data[index];
                data[index] = old + amount * (255 - old) / 255.0f;
            }
        }
    }

    void add(S32 x, S32 y, U32 amount) override {
        if (x < 0 || y < 0)
            return;
        Rect newBounds = bounds;
        if (newBounds.expand(x, y))
            expand(newBounds);
        U32 index = (y - bounds.y) * bounds.width + (x - bounds.x);
        if (index >= data.size()) {
            logE("Invalid selection add");
        } else {
            U32 old = data[index] + amount;
            data[index] = old > 0xFF ? 0xFF : old;
        }
    }

    void subtract(S32 x, S32 y, U32 amount) override {
        if (x < 0 || y < 0)
            return;
        if (!bounds.contains(x, y))
            return;
        U32 index = (y - bounds.y) * bounds.width + (x - bounds.x);
        if (index >= data.size()) {
            logE("Invalid selection subtract");
        } else {
            S32 old = data[index] - amount;
            data[index] = old < 0 ? 0 : old;
        }
    }

    U8 get(S32 x, S32 y) override {
        if (!bounds.contains(x, y))
            return 0;
        U32 index = (y - bounds.y) * bounds.width + (x - bounds.x);
        if (index >= data.size()) {
            logE("Invalid selection get");
            return 0;
        } else {
            return data[index];
        }
    }

    Vector<U32> read(Surface* surface) override {
        Vector<U32> pixels;
        U32 maxY = std::min<U32>(bounds.bottom(), surface->height());
        U32 maxX = std::min<U32>(bounds.right(), surface->width());
        for (U32 y = bounds.y; y < maxY; ++y) {
            U32 index = (y - bounds.y) * bounds.width;
            for (U32 x = bounds.x; x < maxX; ++x) {
                if (data[index++]) {
                    pixels.push_back(surface->getPixelUnsafe(x, y));
                }
            }
        }
        return pixels;
    }

    void write(Surface* surface, Vector<U32>& pixels) override {
        U32 maxY = std::min<U32>(bounds.bottom(), surface->height());
        U32 maxX = std::min<U32>(bounds.right(), surface->width());
        U32 cursor = 0;
        for (U32 y = bounds.y; y < maxY; ++y) {
            U32 index = (y - bounds.y) * bounds.width;
            for (U32 x = bounds.x; x < maxX; ++x) {
                if (data[index++]) {
                    surface->setPixelUnsafe(x, y, pixels[cursor++]);
                }
            }
        }
    }

    void clear() override {
        bounds.width = 0;
        data.resize(0);
    }
};

static Selection::Shared<SelectionImpl> reg{"new"};
