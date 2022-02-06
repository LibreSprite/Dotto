// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/String.hpp>
#include <texttransform/TextTransform.hpp>

class Capitalize : public TextTransform {
public:
    String run(const String& input, const ui::Node* node) override {
        String out;
        out.reserve(input.size());
        bool cap = true;
        for (auto c : input) {
            out.push_back(cap ? std::toupper(c) : c);
            cap = c <= 32;
        }
        return out;
    }
};

static TextTransform::Singleton<Capitalize> reg{"capitalize"};
