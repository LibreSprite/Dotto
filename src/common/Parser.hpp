// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Value.hpp>
#include <fs/File.hpp>

class Parser : public Injectable<Parser> {
public:
    virtual bool canCache() {return true;}
    virtual Value parseFile(std::shared_ptr<fs::File> file) = 0;
};
