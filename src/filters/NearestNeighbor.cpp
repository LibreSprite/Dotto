// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <common/Surface.hpp>
#include <doc/Document.hpp>
#include <filters/Filter.hpp>

class Surface;

class NearestNeighbor : public Filter {
public:
    Property<U32> width{this, "width"};
    Property<U32> height{this, "height"};
    Property<std::shared_ptr<Document>> doc{this, "document"};

    bool forceAllLayers() override {return true;}
    bool forceAllFrames() override {return true;}

    std::shared_ptr<PropertySet> getMetaProperties() override {
        auto meta = Filter::getMetaProperties();

        inject<Document> doc{"activedocument"};
        if (doc) {
            width.value = doc->width();
            height.value = doc->height();
        }

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "number"},
                    {"label", width.name},
                    {"value", width.value}
                }));

        meta->push(std::make_shared<PropertySet>(PropertySet{
                    {"widget", "number"},
                    {"label", height.name},
                    {"value", height.value}
                }));

        return meta;
    }

    void undo() override {
        S32 width = undoData->get<S32>("width");
        S32 height = undoData->get<S32>("height");
        (*doc)->setDocumentSize(width, height);
    }

    void run(std::shared_ptr<Surface> surface) override {
        auto data = surface->getPixels();
        S32 inwidth = surface->width();
        S32 inheight = surface->height();
        S32 outwidth = width;
        S32 outheight = height;
        if (inwidth == outwidth && inheight == outheight)
            return;

        if (!undoData) {
            undoData = std::make_shared<PropertySet>();
            undoData->set("width", inwidth);
            undoData->set("height", inheight);
        }

        surface->resize(outwidth, outheight);

        for (S32 y = 0; y < outheight; ++y) {
            U32 yi = (y * inheight / outheight) * inwidth;
            for (S32 x = 0; x < outwidth; ++x)
                surface->setPixelUnsafe(x, y, data[yi + (x * inwidth / outwidth)]);
        }

        surface->setDirty(surface->rect());
        (*doc)->setDocumentSize(outwidth, outheight);
    }
};

static Filter::Shared<NearestNeighbor> reg{"nearest neighbor"};
