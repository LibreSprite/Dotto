// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/match.hpp>
#include <common/Parser.hpp>
#include <common/System.hpp>
#include <doc/Surface.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Node.hpp>

namespace ui {
    class Image : public Node {
        Property<String> src{this, "src", "", &Image::reload};
        Property<std::shared_ptr<Surface>> surface{this, "surface"};

    public:
        void reload() {
            auto& surface = *this->surface;
            surface.reset();
            if (src->empty())
                return;
            inject<FileSystem> fs;
            auto file = fs->find(*src)->get<File>();
            if (!file || !file->open()) {
                logE("Could not load image ", src);
            } else if (auto parser = inject<Parser>{fs->extension(src)}) {
                surface = parser->parseFile(file);
                if (!surface) logE("Could not parse ", src);
                else {
                    logV("Loaded ", src);
                    match::variant(*surface,
                                   [](Surface256& surface){
                                       logI("Image256 size: ", surface.width(), " x ", surface.height());
                                   },
                                   [](SurfaceRGBA& surface){
                                       logI("ImageRGBA size: ", surface.width(), " x ", surface.height());
                                   });
                }
            } else {
                logE("Unknown file type ", *src);
            }
        }

        void draw(U32 z, Graphics& g) override {
            Node::draw(z, g);
            g.blit(**surface, 10, 10, z);
        }
    };
}

static ui::Node::Shared<ui::Image> img{"image"};
