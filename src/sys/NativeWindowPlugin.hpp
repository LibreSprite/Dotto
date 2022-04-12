// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "common/Value.hpp"
#include "common/inject.hpp"

#if defined(__linux__) && !defined(ANDROID)
#include <X11/Xlib.h>
#endif

#if defined(__WINDOWS__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX   /* don't define min() and max(). */
#define NOMINMAX
#endif
#include <windows.h>
#endif

class NativeWindowPlugin : public Injectable<NativeWindowPlugin> {
public:
#if defined(__linux__) && !defined(ANDROID)
    Display* display = nullptr;
    Window window = 0;
#endif

#if defined(__WINDOWS__)
    HWND window;
    HDC hdc;
    HINSTANCE hinstance;
#endif

    virtual void init() = 0;
};
