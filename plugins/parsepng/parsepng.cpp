#include "API.hpp"
#include <cstdint>
#include <png.h>
#include <cstring>
#include <fmt.hpp>
#include <sys/unistd.h>

SurfaceId loadPNGImage(const char* name) {
    png_image image; /* The control structure used by libpng */
    /* Initialize the 'png_image' structure. */
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    std::vector<std::byte> buffer;

    /* The first argument is the file to read: */
    if (png_image_begin_read_from_file(&image, name) == 0) {
	/* Something went wrong reading or writing the image.  libpng stores a
	 * textual message in the 'png_image' structure:
	 */
	fprintf(stderr, "pngtopng: error: %s\n", image.message);
	return SurfaceId(0);
    }

    uint32_t width{image.width}, height{image.height};

    /* Set the format in which to read the PNG file; this code chooses a
     * simple sRGB format with a non-associated alpha channel, adequate to
     * store most images.
     */
    image.format = PNG_FORMAT_RGBA;

    /* Now allocate enough memory to hold the image in this format; the
     * PNG_IMAGE_SIZE macro uses the information about the image (width,
     * height and format) stored in 'image'.
     */
    // buffer = malloc(PNG_IMAGE_SIZE(image));
    buffer.resize(PNG_IMAGE_SIZE(image));

    /* If enough memory was available, read the image in the desired
     * format, then write the result out to the new file.  'background' is
     * not necessary when reading the image, because the alpha channel is
     * preserved; if it were to be removed, for example if we requested
     * PNG_FORMAT_RGB, then either a solid background color would have to
     * be supplied, or the output buffer would have to be initialized to
     * the actual background of the image.
     *
     * The fourth argument to png_image_finish_read is the 'row_stride' -
     * this is the number of components allocated for the image in each
     * row.  It has to be at least as big as the value returned by
     * PNG_IMAGE_ROW_STRIDE, but if you just allocate space for the
     * default, minimum size, using PNG_IMAGE_SIZE as above, you can pass
     * zero.
     *
     * The final argument is a pointer to a buffer for the colormap;
     * colormaps have exactly the same format as a row of image pixels
     * (so you choose what format to make the colormap by setting
     * image.format).  A colormap is only returned if
     * PNG_FORMAT_FLAG_COLORMAP is also set in image.format, so in this
     * case NULL is passed as the final argument.  If you do want to force
     * all images into an index/color-mapped format, then you can use:
     *
     *    PNG_IMAGE_COLORMAP_SIZE(image)
     *
     * to find the maximum size of the colormap in bytes.
     */
    if (!png_image_finish_read(&image, NULL/*background*/, buffer.data(), 0/*row_stride*/, NULL/*colormap*/))
    {
	/* Calling png_image_free is optional unless the simplified API was
	 * not run to completion.  In this case, if there wasn't enough
	 * memory for 'buffer', we didn't complete the read, so we must
	 * free the image:
	 */
	png_image_free(&image);
	return SurfaceId(0);
    }

    auto surface = createSurface(width, height);
    Surface_write(surface, 0, 0, width, height, (Color*)buffer.data());
    return surface;
}


int main(int argc, const char* argv[]) {
    if (argc < 2) {
	printf("parseobj error: expected 2 arguments, got %d.\n", argc);
	return 1;
    }
    auto ok = loadPNGImage(argv[0]);
    message("{} {:#x} {:#x}", argv[1], getpid(), ok);
    return 0;
}
