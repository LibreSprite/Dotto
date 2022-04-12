// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#if !defined(ANDROID)
#include <png.h>

#include <common/Surface.hpp>
#include <common/Writer.hpp>
#include <log/Log.hpp>

using namespace fs;

class PNGEncoder {
public:
    png_structp png = nullptr;
    png_infop info = nullptr;

    ~PNGEncoder() {
        if (png)
            png_destroy_write_struct(&png, &info);
    }

    bool write(File& file, std::shared_ptr<Surface> surface) {
        png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            logE("Could not init libpng");
            return false;
        }

        info = png_create_info_struct(png);
        if (!info) {
            return false;
        }

        if (setjmp(png_jmpbuf(png))) {
            return false;
        }

        U32 width = surface->width();
        U32 height = surface->height();

        png_set_write_fn(
            png,
            &file,
            +[](png_structp png, png_bytep data, size_t size){
                static_cast<File*>(png_get_io_ptr(png))->write(data, size);
            },
            +[](png_structp png){});

        png_set_IHDR(
            png,
            info,
            width, height,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
            );
        png_write_info(png, info);

        Vector<png_bytep> rowPointers;
        rowPointers.resize(height);
        for (int y = 0; y < height; ++y) {
            rowPointers[y] = reinterpret_cast<png_bytep>(surface->data() + y * width);
        }

        png_write_image(png, rowPointers.data());
        png_write_end(png, nullptr);
        return true;
    }
};

class PngWriter : public SimpleImageWriter {
public:
    bool writeFile(std::shared_ptr<File> file, const Value& data) override {
        return PNGEncoder{}.write(*file, data);
    }
};

static Writer::Shared<PngWriter> png{"png", {"*.png"}};
#endif
