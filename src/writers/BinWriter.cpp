// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/PropertySet.hpp>
#include <common/Writer.hpp>
#include <script/Value.hpp>

class BinWriter : public Writer {
public:
    bool writeFile(std::shared_ptr<fs::File> file, const Value& data) override {
        auto bin = data.get<script::Value::Buffer*>();
        file->write(bin->data(), bin->size());
        return true;
    }
};

static Writer::Shared<BinWriter> bin{"bin", {"*", typeid(script::Value::Buffer*).name()}};
