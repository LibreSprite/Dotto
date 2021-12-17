// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <inject.hpp>
#include <gui/Window.hpp>

class System : public Injectable<System> {
public:
    virtual bool boot() = 0;
    virtual bool run() = 0;
};
