// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/match.hpp>
#include <common/Messages.hpp>
#include <common/Parser.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <common/System.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Graphics.hpp>
#include <gui/Node.hpp>

namespace ui {
    class Image : public Node {
        Property<String> src{this, "src", "", &Image::reload};
        Property<std::shared_ptr<Surface>> surface{this, "surface"};
        Property<Rect> nineSlice{this, "slice"};
        PubSub<msg::Flush> pub{this};

        void reload() {
            auto& surface = *this->surface;
            surface.reset();
            if (src->empty())
                return;
            surface = FileSystem::parse(*src);
            if (!surface)
                logE("Could not load image ", src);
            load({{"surface", surface}});
        }

    public:
        void on(msg::Flush& flush) {
            flush.hold(*surface);
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
