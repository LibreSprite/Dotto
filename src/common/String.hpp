// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <regex>

#include <common/types.hpp>

String tolower(const String& string);

String toupper(const String& string);

String trim(const String& input);

Vector<String> inline split(const String& str, const std::regex& expr) {
    return {std::sregex_token_iterator{str.begin(), str.end(), expr, -1}, std::sregex_token_iterator{}};
}

Vector<String> split(const String& str, const String& sep);

String join(const Vector<String>& str, const String& sep);

bool startsWith(const String& str, const String& prefix);
