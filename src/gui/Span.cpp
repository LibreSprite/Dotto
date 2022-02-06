// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <regex>

#include <common/Config.hpp>
#include <common/Font.hpp>
#include <common/match.hpp>
#include <common/Messages.hpp>
#include <common/Parser.hpp>
#include <common/PubSub.hpp>
#include <common/Surface.hpp>
#include <common/System.hpp>
#include <filters/Filter.hpp>
#include <fs/FileSystem.hpp>
#include <gui/Graphics.hpp>
#include <gui/Node.hpp>

namespace ui {
    class Span : public Node {
        Property<bool> translate{this, "translate", true, &Span::redraw};
        Property<String> fontPath{this, "font", "", &Span::reload};
        Property<Rect> fontPadding{this, "font-padding"};
        Property<String> text{this, "text", "", &Span::redraw};
        Property<String> match{this, "match", "", &Span::redraw};
        Property<String> replacement{this, "replacement", "", &Span::redraw};
        Property<S32> align{this, "align", -1};
        Property<U32> size{this, "size", 12, &Span::redraw};
        Property<Color> color{this, "color", {0xFF, 0xFF, 0xFF}, &Span::redraw};
        Property<Vector<S32>> advance{this, "text-advance"};
        Property<std::shared_ptr<Font>> font{this, "font-ptr", nullptr, &Span::redraw};
        Property<std::shared_ptr<Surface>> surface{this, "surface"};
        Property<String> filterString{this, "filter", "", &Span::changeFilter};
        PubSub<msg::Flush> pub{this};
        inject<Config> config;
        std::shared_ptr<Filter> filter;

        void changeFilter() {
            if (filterString->empty()) {
                filter.reset();
            } else {
                filter = inject<Filter>{*filterString};
            }
            if (filter) {
                filter->load(getPropertySet());
            }
            applyFilter();
        }

        void applyFilter() {
            if (filter && *surface) {
                filter->run(*surface);
            }
        }

        void redraw() {
            std::shared_ptr<Surface> surface;
            Vector<S32> advance;
            if (auto font = *this->font) {
                String str = text;
                if (translate)
                    str = config->translate(str, this);
                if (!match->empty()) {
                    // logI("Match:", *match, " replacement:");
                    try {
                        str = std::regex_replace(str.c_str(), std::regex(match->c_str()), replacement->c_str());
                    } catch(std::regex_error& err) {
                        logE("Regex error: ", err.what());
                    }
                }
                surface = font->print(size, *color, str, fontPadding, advance);
            }
            applyFilter();
            set("surface", surface);
            set("text-advance", advance);
            resize();
        }

        void reload() {
            auto& surface = *this->surface;
            surface.reset();
            if (!fontPath->empty())
                *font = FileSystem::parse(fontPath);
            redraw();
        }

    public:
        void on(msg::Flush& flush) {
            flush.hold(*font);
        }

        void onResize() override {
            if (*surface) {
                localRect.x = 0;
                localRect.y = 0;
                localRect.width = (*surface)->width();
                localRect.height = (*surface)->height();
                if (align == -1) { // left, default
                } else if (align == 0) { // center
                    globalRect.x += globalRect.width/2 - localRect.width/2;
                } else if (align == 1) { // right
                    globalRect.x += globalRect.width - localRect.width;
                }
                if (localRect.width) {
                    F32 factor = globalRect.width / F32(localRect.width);
                    if (factor > 1) {
                        globalRect.width = localRect.width;
                        globalRect.height = localRect.height;
                    } else {
                        globalRect.y += globalRect.height * (1 - factor) / 2;
                        globalRect.height = localRect.height * factor;
                    }
                }
            }
        }

        void draw(S32 z, Graphics& g) override {
            if (*surface) {
                g.blit({
                        .surface = *surface,
                        .source = localRect,
                        .destination = globalRect,
                        .nineSlice = {},
                        .zIndex    = z,
                        .multiply  = multiply
                    });
            }
            Node::draw(z, g);
        }
    };
}

static ui::Node::Shared<ui::Span> span{"span"};
