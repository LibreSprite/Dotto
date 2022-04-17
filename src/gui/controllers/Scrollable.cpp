// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Unit.hpp>

class Scrollable : public ui::Controller {
public:
    Property<bool> enabled{this, "scrollable", true};
    PubSub<> pub{this};

    void attach() override {
        node()->addEventListener<ui::Resize, ui::MouseWheel>(this);
    }

    void scroll(S32 x, S32 y) {
        Rect childBounds;
        for (auto& child : node()->getChildren()) {
            childBounds.expand(child->globalRect);
        }
        Rect padding = *node()->padding;
        Rect bounds = node()->globalRect;

        if (bounds.height >= childBounds.height) {
            padding.y = 0;
        } else if (y < 0 && bounds.bottom() > childBounds.bottom() + y) {
            padding.y = bounds.height - childBounds.height;
        } else if (y > 0 && bounds.top() < childBounds.top() + y) {
            padding.y = 0;
        } else {
            padding.y += y;
        }

        if (bounds.width >= childBounds.width) {
            padding.x = 0;
        } else if (x < 0 && bounds.right() > childBounds.right() + x) {
            padding.x = bounds.width - childBounds.width;
        } else if (x > 0 && bounds.left() < childBounds.left() + x) {
            padding.x = 0;
        } else {
            padding.x += x;
        }

        if (!(padding == *node()->padding)) {
            *node()->padding = padding;
            node()->resize();
        }
    }

    void eventHandler(const ui::Resize& event) {
        if (!*enabled)
            return;
    }

    void eventHandler(const ui::MouseWheel& event) {
        event.cancel = true;
        scroll(event.wheelX * 20, event.wheelY * 20);
    }
};

static ui::Controller::Shared<Scrollable> scrollable{"scrollable"};
