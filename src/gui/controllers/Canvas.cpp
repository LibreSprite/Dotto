// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Messages.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <common/fork_ptr.hpp>
#include <doc/Cell.hpp>
#include <doc/Document.hpp>
#include <doc/Timeline.hpp>
#include <gui/Controller.hpp>
#include <gui/Events.hpp>
#include <gui/Node.hpp>
#include <gui/Texture.hpp>

class Canvas : public ui::Controller {
public:
    PubSub<msg::ModifyCell,
           msg::PostTick> pub{this};

    Property<std::shared_ptr<Surface>> surface{this, "surface"};
    Property<std::shared_ptr<Document>> doc{this, "document", nullptr, &Canvas::setDocument};
    Property<U32> frame{this, "frame", 0, &Canvas::setFrame};
    Property<U32> layer{this, "layer"};

    std::shared_ptr<Cell> activeCell;

    void setDocument() {setFrame();}

    void setFrame() {redraw();}

    void redraw() {
        if (!*doc) {
            return;
        }
        auto& doc = **this->doc;
        auto timeline = doc.currentTimeline();
        if (!timeline)
            return;
        auto cell = timeline->getCell(frame);
        if (!cell)
            return;
        auto surface = cell->getComposite();
        if (!surface || surface == this->surface->get())
            return;
        node()->set("surface", surface->shared_from_this());
    }

    void on(msg::ModifyCell& event) {
        redraw();
    }

    void on(msg::PostTick&) {
        redraw();
    }

    void attach() override {
        redraw();
    }
};

static ui::Controller::Shared<Canvas> canvas{"canvas"};
