// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <algorithm>

#include <common/match.hpp>
#include <common/Color.hpp>
#include <log/Log.hpp>

class ColorOctree {
public:
    using Children = Vector<ColorOctree>;
    using Colors = Vector<std::pair<Color, U32>>;
    static inline constexpr const U32 maxSize = 32;
    std::variant<Colors, Children> data;
    U64 hits = 0;

    Color min, max;
    ColorOctree(const Color& min, const Color& max) : min{min}, max{max} {}
    ColorOctree() = delete;

    void collect(Vector<Color>& colors) {
        Vector<ColorOctree*> nodes;
        nodes.resize(colors.size());
        collect(nodes);
        Colors top;
        top.resize(colors.size());

        for (auto node : nodes) {
            if (!node)
                continue;
            for (auto insert : std::get<Colors>(node->data)) {
                for (auto& prev : top) {
                    if (insert.second > prev.second) {
                        std::swap(insert, prev);
                        if (!insert.second)
                            break;
                    }
                }
            }
        }

        colors.clear();
        for (auto& color : top) {
            colors.push_back(color.first);
        }
    }

    void collect(Vector<ColorOctree*>& nodes) {
        if (hits) {
            auto insert = this;
            auto hits = this->hits;
            for (U32 i = 0, size = nodes.size(); i < size; i++) {
                auto other = nodes[i];
                if (!other || other->hits < hits) {
                    nodes[i] = insert;
                    insert = other;
                    if (other)
                        hits = other->hits;
                    else
                        break;
                }
            }
        }
        match::variant(data,
                       [&](Colors& colors){},
                       [&](Children& children){
                           for (auto& child : children)
                               child.collect(nodes);
                       }
            );
    }

    operator Color () {
        Color ret;
        ret.r = (U32(min.r) + max.r) >> 1;
        ret.g = (U32(min.g) + max.g) >> 1;
        ret.b = (U32(min.b) + max.b) >> 1;
        ret.a = (U32(min.a) + max.a) >> 1;
        match::variant(data,
                       [&](Colors& colors){
                           if (!colors.empty())
                               ret = colors[0].first;
                       },
                       [&](Children& children){}
            );
        return ret;
    }

    void add(const Color& color, U32 amount = 1) {
        match::variant(data,
                       [&](Colors& colors){add(colors, color, amount);},
                       [&](Children& children){add(children, color, amount);}
            );
    }

    void add(Colors& colors, const Color& color, U32 amount) {
        hits += amount;

        for (auto& entry : colors) {
            if (entry.first != color)
                continue;
            entry.second += amount;
            return;
        }

        colors.push_back({color, 1});

        if (colors.size() > maxSize)
            split();
    }

    void add(Children& children, const Color& color, U32 amount) {
        for (auto& child : children) {
            if (color.r < child.min.r) continue;
            if (color.g < child.min.g) continue;
            if (color.b < child.min.b) continue;
            if (color.a < child.min.a) continue;
            if (color.r > child.max.r) continue;
            if (color.g > child.max.g) continue;
            if (color.b > child.max.b) continue;
            if (color.a > child.max.a) continue;
            child.add(color, amount);
            return;
        }
        logE("Could not find tree node for color");
    }

    void split() {
        logI("Splitting color");

        // if (max.r - min.r < 3) return;
        // if (max.g - min.g < 3) return;
        // if (max.b - min.b < 3) return;
        // if (max.a - min.a < 3) return;
        hits = 0;

        auto colors{std::move(std::get<Colors>(data))};
        Vector<ColorOctree> children;
        children.reserve(1 << 4);

        Color cmin, cmax, mid;
        mid.r = (max.r - min.r) / 2 + min.r;
        mid.g = (max.g - min.g) / 2 + min.g;
        mid.b = (max.b - min.b) / 2 + min.b;
        mid.a = (max.a - min.a) / 2 + min.a;

        for (U32 i = 0; i < 1 << 4; ++i) {
            U32 R = (i     ) & 1;
            U32 G = (i >> 1) & 1;
            U32 B = (i >> 2) & 1;
            U32 A = (i >> 3) & 1;

            if (R) {
                cmin.r = mid.r + 1;
                cmax.r = max.r;
            } else {
                cmin.r = min.r;
                cmax.r = mid.r;
            }
            if (G) {
                cmin.g = mid.g + 1;
                cmax.g = max.g;
            } else {
                cmin.g = min.g;
                cmax.g = mid.g;
            }
            if (B) {
                cmin.b = mid.b + 1;
                cmax.b = max.b;
            } else {
                cmin.b = min.b;
                cmax.b = mid.b;
            }
            if (A) {
                cmin.a = mid.a + 1;
                cmax.a = max.a;
            } else {
                cmin.a = min.a;
                cmax.a = mid.a;
            }

            children.emplace_back(cmin, cmax);
        }

        for (auto& color : colors)
            add(children, color.first, color.second);
        data = std::move(children);
    }
};
