// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef USE_SDL2

#include <common/Parser.hpp>
#include <sdl2/SDL2Image.hpp>

using namespace fs;

class SDLImageParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        U32 size = file->size();
        Vector<U8> data;
        data.resize(size);
        file->read(data.data(), size);
        return SDLImage::instance().load(data);
    }
};

static Parser::Shared<SDLImageParser> bmp{"bmp", {"image"}};
static Parser::Shared<SDLImageParser> gif{"gif", {"image"}};
static Parser::Shared<SDLImageParser> jpg{"jpg", {"image"}};
static Parser::Shared<SDLImageParser> jpeg{"jpeg", {"image"}};
static Parser::Shared<SDLImageParser> lbm{"lbm", {"image"}};
static Parser::Shared<SDLImageParser> pcx{"pcx", {"image"}};
// static Parser::Shared<SDLImageParser> png{"png", {"image"}};
static Parser::Shared<SDLImageParser> pnm{"pnm", {"image"}};
static Parser::Shared<SDLImageParser> svg{"svg", {"image"}};
static Parser::Shared<SDLImageParser> tga{"tga", {"image"}};
static Parser::Shared<SDLImageParser> tiff{"tiff", {"image"}};
static Parser::Shared<SDLImageParser> tif{"tif", {"image"}};
static Parser::Shared<SDLImageParser> webp{"webp", {"image"}};
static Parser::Shared<SDLImageParser> xcf{"xcf", {"image"}};
static Parser::Shared<SDLImageParser> xpm{"xpm", {"image"}};
static Parser::Shared<SDLImageParser> xv{"xv", {"image"}};

#endif
