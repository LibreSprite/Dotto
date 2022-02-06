// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/String.hpp>
#include <texttransform/TextTransform.hpp>

class ToLower : public TextTransform {
public:
    String run(const String& input, const ui::Node* node) override {
        return tolower(input);
    }
};

static TextTransform::Singleton<ToLower> reg{"tolower"};
