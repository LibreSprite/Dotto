// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef EMSCRIPTEN

#include <tinyfiledialogs/tinyfiledialogs.h>

#include <common/String.hpp>
#include <fs/FileDialog.hpp>
#include <log/Log.hpp>

class LinuxFileDialog : public FileDialog {
public:
    Vector<const char*> cfilters;

    void init() {
        result.clear();
        cfilters.reserve(filters.size());
        for (auto& filter : filters)
            cfilters.push_back(filter.c_str());
    }

    bool open() {
        init();
        if (auto chosen = tinyfd_openFileDialog(title.c_str(),
                                                defaultPath.c_str(),
                                                cfilters.size(),
                                                cfilters.data(),
                                                filterDescription.c_str(),
                                                allowMultiple)) {
            result = split(chosen, "|");
            return true;
        }
        return false;
    }

    bool save() {
        init();
        if (auto chosen = tinyfd_saveFileDialog(title.c_str(),
                                                defaultPath.c_str(),
                                                cfilters.size(),
                                                cfilters.data(),
                                                filterDescription.c_str())) {
            result = {chosen};
        }
        return true;
    }
};

static FileDialog::Shared<LinuxFileDialog> reg{""};

#endif
