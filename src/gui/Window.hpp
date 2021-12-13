// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <gui/Node.hpp>
#include <common/Color.hpp>

namespace ui {

class Window : public Node {
public:
    Property<String> title{this, "title"};
    Property<bool> maximized{this, "maximized"};
    Property<bool> border{this, "border"};
    Property<S32> x{this, "x"};
    Property<S32> y{this, "y"};
    Property<U32> width{this, "width"};
    Property<U32> height{this, "height"};
    Property<Color> background{this, "background"};
};

}
