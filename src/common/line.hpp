// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/types.hpp>

template<typename Func>
void line(Point a, Point b, Func func) {
    S32 x0 = a.x;
    S32 y0 = a.y;
    S32 x1 = b.x;
    S32 y1 = b.y;
    S32 e;
    S32 dx, dy, j, temp;
    S32 s1,s2;
    bool exchange;
    S32 x,y;

    x = x0;
    y = y0;

    //take absolute value
    if (x1 < x0) {
        dx = x0 - x1;
        s1 = -1;
    } else if (x1 == x0) {
        dx = 0;
        s1 = 0;
    } else {
        dx = x1 - x0;
        s1 = 1;
    }

    if (y1 < y0) {
        dy = y0 - y1;
        s2 = -1;
    } else if (y1 == y0) {
        dy = 0;
        s2 = 0;
    } else {
        dy = y1 - y0;
        s2 = 1;
    }

    exchange = false;

    if (dy > dx) {
        temp = dx;
        dx = dy;
        dy = temp;
        exchange = true;
    }

    e = (dy * 2) - dx;

    for (j=0; j<=dx; j++) {
        func({x, y});

        if (e >= 0) {
            if (exchange) {
                x += s1;
            } else {
                y += s2;
            }
            e -= dx * 2;
        }

        if (exchange) {
            y += s2;
        } else {
            x += s1;
        }
        e += dy * 2;
    }
}
