// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <common/inject.hpp>
#include <common/PropertySet.hpp>

namespace ui {
    class Node;

    class Controller : public Injectable<Controller>,
                       public Serializable,
                       public std::enable_shared_from_this<Controller> {
        Property<Node*> _node{this, "node", nullptr, &Controller::attach};

    protected:
        Node* node() {
            return *_node;
        }

    public:
        virtual void attach() {}
        virtual void detach();

        void init(const PropertySet& properties) {load(properties);}
    };
}
