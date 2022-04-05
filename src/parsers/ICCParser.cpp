// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef LCMS2_SUPPORT

#include <lcms2.h>

#include <common/ColorProfile.hpp>
#include <common/Parser.hpp>
#include <common/Surface.hpp>
#include <fs/FileSystem.hpp>

using namespace fs;

struct ICCProfile {
    cmsHPROFILE profile;

    operator cmsHPROFILE () {
        return profile;
    }

    ~ICCProfile() {
        if (profile)
            cmsCloseProfile(profile);
    }
};

struct ICCTransform {
    cmsHTRANSFORM transform = nullptr;

    ICCTransform& operator = (cmsHTRANSFORM transform) {
        this->transform = transform;
        return *this;
    }

    operator cmsHTRANSFORM () {
        return transform;
    }

    ~ICCTransform() {
        if (transform)
            cmsDeleteTransform(transform);
    }
};

class ColorProfileImpl : public ColorProfile {
public:
    ICCTransform transform;

    ColorProfileImpl(File& file) {
        auto srgbEntity = inject<FileSystem>{}->find("%appdata/icc/sRGB.icc");
        if (!srgbEntity || !srgbEntity->isFile())
            return;
        auto srgb = srgbEntity->get<File>();
        if (!srgb->open())
            return;

        Vector<U8> bytes;

        auto size = srgb->size();
        bytes.resize(size);
        srgb->read(bytes.data(), size);
        ICCProfile in{cmsOpenProfileFromMem(bytes.data(), size)};
        if (!in)
            return;

        size = file.size();
        bytes.resize(size);
        file.read(bytes.data(), size);
        ICCProfile out{cmsOpenProfileFromMem(bytes.data(), size)};
        if (!out)
            return;

        transform = cmsCreateTransform(in, TYPE_RGBA_8, out, TYPE_RGBA_8, INTENT_PERCEPTUAL, 0);
    }

    bool apply(Surface* surface) override {
        if (!transform || !surface)
            return false;
        cmsDoTransform(transform, surface->data(), surface->data(), surface->width() * surface->height());
        surface->setDirty(surface->rect());
        return true;
    }
};

class ICCParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        auto profile = std::make_shared<ColorProfileImpl>(*file);
        return profile->transform ? std::static_pointer_cast<ColorProfile>(profile) : nullptr;
    }
};

static Parser::Shared<ICCParser> icc{"icc"};

#endif
