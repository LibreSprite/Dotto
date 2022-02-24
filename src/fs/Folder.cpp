// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <filesystem>

#include <fs/Folder.hpp>

void Folder::forEach(std::function<void(std::shared_ptr<FSEntity>)> callback) {
    if (!this->path.empty()) {
        try {
            std::filesystem::path path{this->path};
            for (const auto& entry : std::filesystem::directory_iterator{path}) {
                String type;
                if (entry.is_directory()) {
                    type = "dir";
                } else if (entry.is_regular_file()) {
                    type = "std";
                } else continue;
                callback(getChild(entry.path().filename().string(), type));
            }
        } catch (std::filesystem::filesystem_error&) {}
    } else {
        for (auto& entry : children) {
            callback(getChild(entry.first, ""));
        }
    }
}
