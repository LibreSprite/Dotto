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

        auto out = &swap[yoffset * rect.width + xoffset];
        auto in = &data[0];
        U32 width = bounds.width;
        for (U32 y = 0; y < bounds.height; ++y) {
            for (U32 x = 0; x < width; ++x) {
                out[x] = in[x];
            }
            out += rect.width;
            in += width;
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
        if (amount == 0)
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
        if (amount == 0)
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
        S32 minY = std::max<S32>(bounds.y, 0);
        S32 maxY = std::min<S32>(bounds.bottom(), surface->height());
        S32 minX = std::max<S32>(bounds.x, 0);
        S32 maxX = std::min<S32>(bounds.right(), surface->width());
        for (S32 y = minY; y < maxY; ++y) {
            U32 index = (y - bounds.y) * bounds.width;
            if (bounds.x < 0) {
                index -= bounds.x;
            }
            for (S32 x = minX; x < maxX; ++x) {
                if (data[index++]) {
                    pixels.push_back(surface->getPixelUnsafe(x, y));
                }
            }
        }
        return pixels;
    }

    void write(Surface* surface, Vector<U32>& pixels) override {
        S32 minY = std::max<S32>(bounds.y, 0);
        S32 maxY = std::min<S32>(bounds.bottom(), surface->height());
        S32 minX = std::max<S32>(bounds.x, 0);
        S32 maxX = std::min<S32>(bounds.right(), surface->width());
        U32 cursor = 0;
        for (S32 y = minY; y < maxY; ++y) {
            U32 index = (y - bounds.y) * bounds.width;
            if (bounds.x < 0) {
                index -= bounds.x;
            }
            for (S32 x = minX; x < maxX; ++x) {
                if (data[index++]) {
                    surface->setPixelUnsafe(x, y, pixels[cursor++]);
                }
            }
        }
    }

    bool empty() override {
        return data.empty();
    }

    void clear() override {
        bounds.width = 0;
        bounds.height = 0;
        data.resize(0);
    }
};

static Selection::Shared<SelectionImpl> reg{"new"};
