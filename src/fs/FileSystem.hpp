// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <inject.hpp>

class FileSystem : public Injectable<FileSystem> {
public:
    Provides p{this};


};
