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
        PubSub<msg::Flush> pub{this};
        Property<Rect> nineSlice{this, "slice"};

    public:
        void on(msg::Flush& flush) {
            flush.hold(*surface);
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
