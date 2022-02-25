// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <qoi/qoi.h>

#include <common/Parser.hpp>
#include <common/Surface.hpp>
#include <log/Log.hpp>

using namespace fs;

class QoiParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        int size = file->size();
        Vector<U8> data;
        data.resize(size);
        file->read(data.data(), size);

        qoi_desc desc;
        std::unique_ptr<void, void(*)(void*)> pixels(qoi_decode(data.data(), size, &desc, 4), free);
        if (!pixels)
            return nullptr;

        auto surface = std::make_shared<Surface>();
        surface->resize(desc.width, desc.height);
        surface->setDirty();
        auto out = surface->data();
        for (U32 i = 0; i < desc.width * desc.height; ++i)
            out[i] = reinterpret_cast<Surface::PixelType*>(pixels.get())[i];

        return surface;
    }
};

static Parser::Shared<QoiParser> qoi{"qoi", {"image"}};
