// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/String.hpp>
#include <texttransform/TextTransform.hpp>

class Round : public TextTransform {
public:
    String run(const String& input, const ui::Node* node) override {
        return tostring(F64(S64(std::atof(input.c_str()) + 0.5)));
    }
};

static TextTransform::Singleton<Round> reg{"round"};
