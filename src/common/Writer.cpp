// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <fs/FileSystem.hpp>
#include <common/Writer.hpp>

bool Writer::writeFile(const String& path, const Value& data) {
    auto fsentity = inject<FileSystem>{}->find(path);
    if (!fsentity)
        return false;
    auto file = fsentity->get<File>();
    if (!file)
        return false;
    if (!file->open({.write=true, .create=true}))
        return false;
    return writeFile(file, data);
}

bool SimpleImageWriter::writeFile(const String& path, const Value& data) {
    if (data.has<std::shared_ptr<Surface>>())
        return Writer::writeFile(path, data);

    if (!data.has<std::shared_ptr<Document>>())
        return false;

    bool result = true;
    auto doc = data.get<std::shared_ptr<Document>>();
    auto timeline = doc->currentTimeline();
    auto layerCount = timeline->layerCount();
    auto frameCount = timeline->frameCount();
    if (layerCount == 1 && frameCount == 1) {
        auto cell = timeline->getCell(0, 0);
        if (!cell) {
            logE("Empty image");
            return false;
        }
        return Writer::writeFile(path, cell->getComposite()->shared_from_this());
    }
    String basePath = path;
    auto dot = basePath.rfind(".");
    String extension = basePath.substr(dot + 1);
    basePath = basePath.substr(0, dot);
    for (U32 frame = 0; frame < frameCount; ++frame) {
        for (U32 layer = 0; layer < layerCount; ++layer) {
            auto cell = timeline->getCell(frame, layer);
            if (!cell)
                continue;
            String cellPath = basePath +
                "-F" + std::to_string(frame) +
                "-L" + std::to_string(layer) +
                "." + extension;
            result &= Writer::writeFile(cellPath, cell->getComposite()->shared_from_this());
        }
    }
    return result;
}
