// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gui/Controller.hpp>
#include <gui/Node.hpp>

namespace ui {

    void Controller::detach() {
        if (_node)
            _node->removeEventListeners(this);
        _node = nullptr;
    }

}
