// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>

namespace ui {
    class Node;

    class Controller : public Injectable<Controller>, public std::enable_shared_from_this<Controller> {
        Node* _node = nullptr;

    protected:
        Node* node() {
            return _node;
        }

    public:
        virtual void attach(Node* node){
            if (!_node)
                _node = node;
        }

        virtual void detach();
    };
}
