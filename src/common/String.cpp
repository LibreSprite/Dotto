// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <regex>

#include <common/String.hpp>

String tostring(U64 n, U32 base) {
    if (!n)
        return "0";
    if (base > 36)
        base = 36;
    String ret;
    while (n) {
        ret += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[n % base];
        n /= base;
    }
    return ret;
}

String tostring(F32 n) {
    String str = std::to_string(n);
    auto dot = str.find(".");
    while (str.back() == '0' && str.size() > dot + 2)
        str.pop_back();
    return str;
}

String tostring(F64 n) {
    String str = std::to_string(n);
    auto dot = str.find(".");
    while (str.back() == '0' && str.size() > dot + 2)
        str.pop_back();
    return str;
}

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
    for (U8 c : input) {
        if (c > ' ')
            break;
        start++;
    }

    if (start == end)
        return "";

    while (U8(input[end - 1]) <= ' ')
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

bool startsWith(const String& str, const String& prefix) {
    if (str.size() < prefix.size())
        return false;
    for (std::size_t i = 0, size = prefix.size(); i < size; ++i) {
        if (str[i] != prefix[i])
            return false;
    }
    return true;
}
