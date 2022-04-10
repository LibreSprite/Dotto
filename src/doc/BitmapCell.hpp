// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <doc/Cell.hpp>

class BitmapCell  : public Cell {
public:
    virtual Selection* getSelection() = 0;
};
