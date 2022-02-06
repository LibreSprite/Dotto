// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/String.hpp>
#include <texttransform/TextTransform.hpp>

class ToUpper : public TextTransform {
public:
    String run(const String& input, const ui::Node* node) override {
        return toupper(input);
    }
};

static TextTransform::Singleton<ToUpper> reg{"toupper"};
