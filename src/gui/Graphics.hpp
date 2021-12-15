// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <doc/Surface.hpp>

class Graphics {
public:
    virtual void blit(Surface& surface, S32 x, S32 y, S32 z){}
};
