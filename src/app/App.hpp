// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>

class App : public Injectable<App> {
public:
    virtual void boot(int argc, const char* argv[]) = 0;
    virtual bool run() = 0;
};
