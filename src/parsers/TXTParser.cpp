// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/inject.hpp>
#include <common/Parser.hpp>
#include <log/Log.hpp>

using namespace fs;

class TXTParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        return file->readTextFile();
    }
};

static Parser::Shared<TXTParser> txt{"txt"};
