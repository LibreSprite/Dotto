// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>

class Window : public Injectable<Window>, public std::enable_shared_from_this<Window>, Serializable {
public:
    Property<String> title{this, "title"};
    Property<bool> maximized{this, "maximized"};
    Property<bool> border{this, "border"};
    Property<U32> width{this, "width"};
    Property<U32> height{this, "height"};

    virtual bool init(const PropertySet& properties) {
        load(properties);
        return true;
    }
};
