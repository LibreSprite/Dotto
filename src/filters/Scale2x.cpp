// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Surface.hpp>
#include <doc/Document.hpp>
#include <filters/Filter.hpp>

class Surface;

class Scale2X : public Filter {
public:
    Property<std::shared_ptr<Document>> doc{this, "document"};

    bool forceAllLayers() override {return true;}
    bool forceAllFrames() override {return true;}

    String category() override {return "resize";}

    std::shared_ptr<PropertySet> getMetaProperties() override {return nullptr;}

    void undo() override {
        (*doc)->setDocumentSize((*doc)->width()/2, (*doc)->height()/2);
    }

    void run(std::shared_ptr<Surface> surface) override {
        auto data = surface->getPixels();
        S32 width = surface->width();
        S32 height = surface->height();
        S32 outwidth = width * 2;
        S32 outheight = height * 2;

        if (!undoData) {
            undoData = std::make_shared<PropertySet>();
        }

        surface->resize(outwidth, outheight);

        Surface::PixelType E0, E1, E2, E3, u, l, c, r, d;

        for (S32 y = 0; y < height; ++y) {
            for (S32 x = 0; x < width; ++x) {
                u = data[(std::max<S32>(y - 1, 0) * width) + x];
                l = data[y * width + std::max<S32>(0, x - 1)];
                c = data[y * width + x];
                r = data[y * width + std::min<S32>(width - 1, x + 1)];
                d = data[(std::min<S32>(height - 1, y + 1) * width) + x];

                E0 = l == u && u != r && l != d ? l : c;
                E1 = r == u && u != l && r != d ? r : c;
                E2 = l == d && u != l && r != d ? l : c;
                E3 = r == d && u != r && l != d ? r : c;

                surface->setPixelUnsafe(x * 2    , y * 2    , E0);
                surface->setPixelUnsafe(x * 2 + 1, y * 2    , E1);
                surface->setPixelUnsafe(x * 2    , y * 2 + 1, E2);
                surface->setPixelUnsafe(x * 2 + 1, y * 2 + 1, E3);
            }
        }

        surface->setDirty(surface->rect());
        (*doc)->setDocumentSize(outwidth, outheight);
    }
};

static Filter::Shared<Scale2X> reg{"scale2x"};
