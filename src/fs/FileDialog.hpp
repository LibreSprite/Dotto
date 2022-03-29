// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>

class FileDialog : public Injectable<FileDialog> {
public:
    using Callback = std::function<void(const Vector<String>&)>;

    String title;
    String defaultPath;
    Vector<String> filters;
    String filterDescription;
    bool allowMultiple = false;

    virtual void open(Callback&&) = 0;
    virtual void save(Callback&&) = 0;
};
