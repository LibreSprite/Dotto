// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <regex>

#include <common/String.hpp>

String tolower(const String& string) {
    String out;
    out.reserve(string.size());
    for (auto c : string)
        out.push_back(std::tolower(c));
    return out;
}

String toupper(const String& string) {
    String out;
    out.reserve(string.size());
    for (auto c : string)
        out.push_back(std::toupper(c));
    return out;
}

String trim(const String& input) {
    std::size_t start = 0;
    std::size_t end = input.size();
    for (auto c : input) {
        if (c > ' ')
            break;
        start++;
    }

    if (start == end)
        return "";

    while (input[end - 1] <= ' ')
        end--;

    return input.substr(start, end - start);
}

Vector<String> split(const String& str, const String& sep) {
    Vector<String> out;
    U32 pos = 0;
    std::size_t brk;
    while (true) {
        brk = str.find(sep, pos);
        if (brk == String::npos) {
            out.push_back(str.substr(pos));
            break;
        }
        out.push_back(str.substr(pos, brk - pos));
        pos = brk + sep.size();
    }
    return out;
}

String join(const Vector<String>& string, const String& sep) {
    bool first = true;
    std::size_t total = 0;
    for (auto& part : string) {
        if (!first)
            total += sep.size();
        first = false;
        total += part.size();
    }

    String acc;
    acc.reserve(total);
    first = true;
    for (auto& part : string) {
        if (!first)
            acc += sep;
        first = false;
        acc += part;
    }

    return acc;
}
