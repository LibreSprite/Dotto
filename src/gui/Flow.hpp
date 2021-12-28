// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/Rect.hpp>
#include <gui/Unit.hpp>

namespace ui {
    class Node;

    class Flow : public Injectable<Flow>, public std::enable_shared_from_this<Flow> {
    protected:
        virtual void absolute(std::shared_ptr<Node> child, Rect& parentRect);
    public:
        virtual void update(Vector<std::shared_ptr<Node>>&, Rect& parentRect) = 0;
    };
}
