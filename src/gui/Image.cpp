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
        Property<String> fit{this, "fit"};
        Property<std::shared_ptr<Surface>> surface{this, "surface"};
        Property<Rect> nineSlice{this, "slice"};
        PubSub<msg::Flush> pub{this};

        void reload() {
            auto& surface = *this->surface;
            surface.reset();
            if (src->empty())
                return;

            std::cmatch match;
            std::regex_match(src->c_str(), match, std::regex("^new:([0-9]+)x([0-9]+)(?::(.*))?"));
            if (!match.empty()) {
                U32 width = std::atoi(match[1].str().c_str());
                U32 height = std::atoi(match[2].str().c_str());

                if (!width || !height)
                    return;

                surface = std::make_shared<Surface>();
                surface->resize(width, height);

                if (match.size() == 4) {
                    auto color = Color{match[3].str()}.toU32();
                    auto data = surface->data();
                    for (U32 i = 0; i < width * height; ++i) {
                        data[i] = color;
                    }
                    surface->setDirty();
                }
            } else {
                surface = FileSystem::parse(*src);
            }

            if (!surface)
                logE("Could not load image ", src);
            load({{"surface", surface}});
        }

    public:
        void on(msg::Flush& flush) {
            flush.hold(*surface);
        }

        void draw(S32 z, Graphics& g) override {
            if (*surface && (*surface)->dataSize()) {
                U32 width = (*surface)->width();
                U32 height = (*surface)->height();
                Rect rect{0, 0, width, height};
                if (*fit == "") { // stretch
                } else if (*fit == "tile") {
                    F32 wf = static_cast<F32>(localRect.width) / width;
                    F32 hf = static_cast<F32>(localRect.height) / height;
                    rect.width *= wf;
                    rect.height *= hf;
                } else if (*fit == "fit") {
                    F32 wf = static_cast<F32>(width) / localRect.width;
                    F32 hf = static_cast<F32>(height) / localRect.height;
                    F32 f = std::max(wf, hf);
                    rect.x = rect.width/2 - static_cast<S32>(localRect.width*f)/2;
                    rect.y = rect.height/2 - static_cast<S32>(localRect.height*f)/2;
                    rect.width = localRect.width * f;
                    rect.height = localRect.height * f;
                } else if (*fit == "cover") {
                    F32 wf = static_cast<F32>(globalRect.width) / width;
                    F32 hf = static_cast<F32>(globalRect.height) / height;
                    if (wf > hf) {
                        height = (height / wf) * hf;
                        rect.y = rect.height/2 - height/2;
                    } else {
                        width = (width / hf) * wf;
                        rect.x = rect.width/2 - width/2;
                    }
                    rect.width = width;
                    rect.height = height;
                }
                g.blit({
                        .surface = *surface,
                        .source = rect,
                        .destination = globalRect,
                        .nineSlice = nineSlice,
                        .zIndex = z,
                        .multiply = multiply,
                        .debug = debug
                    });
            } else {
                g.blit({
                        .surface = nullptr,
                        .source = localRect,
                        .destination = globalRect,
                        .nineSlice = nineSlice,
                        .zIndex = z,
                        .multiply = multiply,
                        .debug = debug
                    });
            }
            Node::draw(z, g);
        }
    };
}

static ui::Node::Shared<ui::Image> img{"image"};
