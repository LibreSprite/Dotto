// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <regex>

#include <common/Config.hpp>
#include <common/String.hpp>
#include <texttransform/TextTransform.hpp>

class ToUpper : public TextTransform {
public:
    inject<Config> config;
    String run(const String& input, const ui::Node* node) override {
        return toupper(input);
    }
};

static TextTransform::Singleton<ToUpper> reg{"toupper"};
