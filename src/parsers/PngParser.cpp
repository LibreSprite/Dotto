// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <png.h>

#include <common/Parser.hpp>
#include <common/Surface.hpp>
#include <log/Log.hpp>

class PNGReader {
public:

    U32 width;
    U32 height;
    png_byte colorType;
    png_byte bitDepth;
    png_bytep *rowPointers = nullptr;

    png_structp png = nullptr;
    png_infop info = nullptr;

    ~PNGReader() {
        if (png)
            png_destroy_read_struct(&png, &info, nullptr);

        if (rowPointers) {
            for(U32 y = 0; y < height; y++)
                free(rowPointers[y]);
            free(rowPointers);
        }
    }

    std::shared_ptr<Surface> read(File& file) {
        png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            logE("Could not init libpng");
            return {};
        }

        info = png_create_info_struct(png);
        if (!info) {
            logE("Could not create info struct");
            return {};
        }

        if (setjmp(png_jmpbuf(png))) {
            logE("An error occurred");
            return {};
        }

        png_set_read_fn(png, &file, +[](png_structp png, png_bytep data, png_size_t length) {
            static_cast<File*>(png_get_io_ptr(png))->read(data, length);
        });

        png_read_info(png, info);

        width     = png_get_image_width(png, info);
        height    = png_get_image_height(png, info);
        colorType = png_get_color_type(png, info);
        bitDepth  = png_get_bit_depth(png, info);

        // Read any colorType into 8bit depth, RGBA format.
        // See http://www.libpng.org/pub/png/libpng-manual.txt

        if (bitDepth == 16)
            png_set_strip_16(png);

        if (colorType == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);

        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
            png_set_expand_gray_1_2_4_to_8(png);

        if (png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);

        // These colorType don't have an alpha channel then fill it with 0xff.
        if (colorType == PNG_COLOR_TYPE_RGB ||
            colorType == PNG_COLOR_TYPE_GRAY ||
            colorType == PNG_COLOR_TYPE_PALETTE)
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

        if (colorType == PNG_COLOR_TYPE_GRAY ||
            colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);

        png_read_update_info(png, info);

        rowPointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
        U32 stride = png_get_rowbytes(png,info);
        for (U32 y = 0; y < height; y++) {
            rowPointers[y] = static_cast<png_byte*>(malloc(stride));
        }

        png_read_image(png, rowPointers);

        auto surface = std::make_shared<Surface>(Surface{});
        surface->resize(width, height);
        auto data = surface->data();
        for (U32 y = 0; y < height; y++) {
            std::memcpy(&data[y * width], rowPointers[y], stride);
        }

        return surface;
    }
};

class PngParser : public Parser {
public:
    Value parseFile(std::shared_ptr<File> file) override {
        return PNGReader{}.read(*file);
    }
};

static Parser::Shared<PngParser> png{"png", {"image"}};
