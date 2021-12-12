#include <common/String.hpp>

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
