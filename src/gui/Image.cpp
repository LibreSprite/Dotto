// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/match.hpp>
#include <common/Messages.hpp>
#include <common/Parser.hpp>
#include <common/PubSub.hpp>
#include <common/System.hpp>
#include <doc/Surface.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Graphics.hpp>
#include <gui/Node.hpp>

namespace ui {
    class Image : public Node {
        Property<String> src{this, "src", "", &Image::reload};
        Property<std::shared_ptr<Surface>> surface{this, "surface"};
        Property<Rect> nineSlice{this, "slice"};
        PubSub<msg::Flush> pub{this};

    public:
        Image() {
            addEventListener<ui::MouseEnter,
                             ui::MouseLeave,
                             ui::Click,
                             ui::Focus,
                             ui::Blur>(this);
        }

        void on(msg::Flush& flush) {
            flush.hold(*surface);
        }

        void eventHandler(const ui::MouseLeave&) {
            logI("Move Leave ", src);
        }

        void eventHandler(const ui::MouseEnter&) {
            logI("Move Enter ", src);
        }

        void eventHandler(const ui::Click&) {
            logI("Click ", src);
        }

        void eventHandler(const ui::Blur&) {
            logI("Blur ", src);
        }

        void eventHandler(const ui::Focus&) {
            logI("Focus ", src);
        }


        void reload() {
            auto& surface = *this->surface;
            surface.reset();
            if (src->empty())
                return;
            surface = inject<FileSystem>{}->find(*src)->parse();
            if (!surface)
                logE("Could not load image ", src);
        }

        void draw(S32 z, Graphics& g) override {
            if (*surface) {
                g.blit({
                        .surface = *surface,
                        .source = localRect,
                        .destination = globalRect,
                        .nineSlice = nineSlice,
                        .zIndex = z
                    });
            }
            Node::draw(z, g);
        }
    };
}

static ui::Node::Shared<ui::Image> img{"image"};
