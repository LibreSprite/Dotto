// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <regex>

#include <texttransform/TextTransform.hpp>
#include <gui/Node.hpp>

class Replace : public TextTransform {
public:
    String run(const String& input, const ui::Node* node) override {
        String match, replacement;
        auto& properties = node->getPropertySet();
        if (!properties.get("match", match))
            return input;
        if (!properties.get("replacement", replacement))
            return input;

        try {
            return std::regex_replace(input.c_str(), std::regex(match.c_str()), replacement.c_str());
        } catch(std::regex_error& err) {
            logE("Regex error: ", err.what());
        }
        return input;
    }
};

static TextTransform::Singleton<Replace> reg{"replace"};
