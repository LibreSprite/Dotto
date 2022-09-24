// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/PropertySet.hpp>
#include <common/Writer.hpp>
#include <script/Value.hpp>

class TxtWriter : public Writer {
public:
    bool writeFile(std::shared_ptr<fs::File> file, const Value& data) override {
        auto txt = data.get<String>();
        file->write(txt.data(), txt.size());
        return true;
    }
};

static Writer::Shared<TxtWriter> txt{"txt", {"*", typeid(String).name()}};
