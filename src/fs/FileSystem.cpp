// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <memory>

#include <common/String.hpp>
#include <fs/File.hpp>
#include <fs/FileSystem.hpp>
#include <fs/Folder.hpp>

bool FileSystem::boot() {
    if (auto root = std::dynamic_pointer_cast<RootFolder>(this->root.shared())) {
        return root->boot();
    }
    return false;
}

String FileSystem::extension(const String &path) {
    auto it = path.find_last_of('.');
    if (it == String::npos)
        return "";
    return tolower(path.substr(it + 1));
}

Vector<String> FileSystem::splitPath(const String& path) {
    auto parts = split(path, std::regex("[/\\\\]"));
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

std::shared_ptr<FSEntity> FileSystem::find(const String& path) {
    auto parts = splitPath(path);
    std::shared_ptr<FSEntity> node = root;
    for (std::size_t index = 0, max = parts.size(); node && index < max; ++index) {
        auto& part = parts[index];
        bool isFilePart = index == max - 1;
        if (!node->isFolder()) {
            if (!isFilePart)
                node.reset();
            break;
        }
        auto parent = std::static_pointer_cast<Folder>(node);
        node = parent->getChild(part, isFilePart ? "std" : "dir");
        if (isFilePart && node->isFile()) {
            break;
        }
    }
    return node;
}
