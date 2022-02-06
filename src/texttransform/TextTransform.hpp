// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>

namespace ui {
    class Node;
}

class TextTransform : public Injectable<TextTransform>, public std::enable_shared_from_this<TextTransform> {
public:
    virtual String run(const String& input, const ui::Node*) = 0;
};
