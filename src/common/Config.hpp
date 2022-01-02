// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>

namespace ui {
    class Node;
}

class Config : public Injectable<Config> {
public:
    std::shared_ptr<PropertySet> properties;
    std::shared_ptr<PropertySet> language;
    virtual bool boot() = 0;
    virtual String translate(const String& str, const ui::Node* context = nullptr) = 0;
};
