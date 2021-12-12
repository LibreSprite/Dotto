// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "fs/File.hpp"
#include <fs/FileSystem.hpp>
#include <common/String.hpp>
#include <memory>

Vector<String> FileSystem::splitPath(const String& path) {
    auto parts = split(path, Folder::separator);
    for (auto it = parts.begin(); it != parts.end();) {
        if (*it == ".") {
            it = parts.erase(it);
            continue;
        }
        if (it != parts.begin()) {
            if (*it == "..") {
                it = parts.erase(it);
                it = parts.erase(it - 1);
                continue;
            }
            if (*it == "") {
                it = parts.erase(it);
                continue;
            }
        }
        ++it;
    }
    return parts;
}

std::shared_ptr<File> FileSystem::open(const String& path, const FileOpenSettings& settings) {
    auto parts = splitPath(path);
    std::shared_ptr<FSEntity> node = root;
    for (std::size_t index = 0, max = parts.size(); node && index < max; ++index) {
        auto& part = parts[index];
        bool isFilePart = index == max - 1;
        if (!node->isFolder())
            break;
        auto parent = std::static_pointer_cast<Folder>(node);
        node = parent->getChild(part, isFilePart ? "std" : "dir");
        if (isFilePart && node->isFile()) {
            break;
        }
    }

    std::shared_ptr<File> file;
    if (!node || !node->isFile()) {
        node = inject<FSEntity>{"std"};
        file = std::static_pointer_cast<File>(node);
        file->init(path);
    } else {
        file = std::static_pointer_cast<File>(node);
    }

    file->open(settings);
    return file;
}
