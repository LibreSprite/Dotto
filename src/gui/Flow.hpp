// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <gui/Rect.hpp>
#include <gui/Unit.hpp>

namespace ui {
    class Node;

    class Flow : public Injectable<Flow>, public std::enable_shared_from_this<Flow> {
    public:
        virtual void update(Vector<std::shared_ptr<Node>>&, ui::Rect& parentRect) = 0;
    };
}
