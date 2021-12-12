// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <fs/File.hpp>
#include <inject.hpp>
#include <Value.hpp>

class Parser : public Injectable<Parser> {
public:
    virtual Value parseFile(std::shared_ptr<File> file) = 0;
};
