// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Surface.hpp>

class Font {
public:
    virtual std::shared_ptr<Surface> print(U32 size, const Color& color, const String& string) = 0;
};
