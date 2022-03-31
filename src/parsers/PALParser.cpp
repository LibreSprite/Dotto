// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <doc/Palette.hpp>
#include <log/Log.hpp>

using namespace fs;

class PalParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        auto lines = split(trim(file->readTextFile()), "\n");
        if (lines.size() < 3 || trim(lines[0]) != "JASC-PAL")
            return nullptr;
        inject<Palette> palette{"new"};
        U32 i = 2;
        S32 colorCount = std::min<S32>(atoi(lines[i++].c_str()), lines.size() - 3);
        Vector<String> rgb;
        Color color;
        for (; colorCount; --colorCount) {
            rgb = split(lines[i++], " ");
            if (rgb.size() < 3)
                continue;
            color.r = atoi(rgb[0].c_str());
            color.g = atoi(rgb[1].c_str());
            color.b = atoi(rgb[2].c_str());
            palette->push(color);
        }
        return palette.shared();
    }
};

static Parser::Shared<PalParser> pal{"pal", {"palette"}};
