// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <qoi/qoi.h>

#include <common/Surface.hpp>
#include <common/Writer.hpp>
#include <doc/Document.hpp>
#include <log/Log.hpp>

using namespace fs;

class QoiWriter : public SimpleImageWriter {
public:
    bool writeFile(std::shared_ptr<File> file, const Value& data) override {
        std::shared_ptr<Surface> surface = data;
        if (!surface)
            return false;
        qoi_desc desc;
        desc.width = surface->width();
        desc.height = surface->height();
        desc.channels = 4;
        desc.colorspace = QOI_SRGB;
        int size = 0;
        std::unique_ptr<void, void(*)(void*)> out(qoi_encode(surface->data(), &desc, &size), free);
        return out && file->write(out.get(), size) == size;
    }
};

static Writer::Shared<QoiWriter> qoi{"qoi", {
        "*.qoi",
        typeid(std::shared_ptr<Surface>).name(),
        typeid(std::shared_ptr<Document>).name()
    }
};
