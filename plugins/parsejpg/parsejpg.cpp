#include "API.hpp"
#include <cstddef>
#include <fmt.hpp>

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <unistd.h>
#include <vector>

#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>

using namespace std::literals;

struct ErrorHandler {
  struct jpeg_error_mgr pub;    /* "public" fields */
  jmp_buf setjmp_buffer;        /* for return to caller */
};

void onError(j_common_ptr cinfo) {
    // cinfo->err really points to a my_error_mgr struct, so coerce pointer
    auto handler = reinterpret_cast<ErrorHandler*>(cinfo->err);

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);

    /* Return control to the setjmp point */
    longjmp(handler->setjmp_buffer, 1);
}

SurfaceId readImageJPG(const char* name) {
    JSAMPARRAY buffer = NULL;     /* Output row buffer */
    J12SAMPARRAY buffer12 = NULL; /* 12-bit output row buffer */
    jpeg_decompress_struct cinfoi;
    auto cinfo = &cinfoi;
    ErrorHandler errorHandler;
    auto file = fopen(name, "rb");
    if (!file) {
	log("Could not open jpeg file: {}", name);
	return SurfaceId(0);
    }
    log("Decoding jpeg {}", name);
    cinfo->err = jpeg_std_error(&errorHandler.pub);
    errorHandler.pub.error_exit = onError;

    /* Establish the setjmp return context for onError to use. */
    if (setjmp(errorHandler.setjmp_buffer)) {
	/* If we get here, the JPEG code has signaled an error.
	 * We need to clean up the JPEG object, close the input file, and return.
	 */
	jpeg_destroy_decompress(cinfo);
	fclose(file);
        log("Decoding jpeg {} ERROR", name);
	return SurfaceId(0);
    }

    #define STEP log("Decoding jpeg LINE {}", __LINE__)

    STEP;

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(cinfo);

    STEP;

    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(cinfo, file);

    STEP;

    /* Step 3: read file parameters with jpeg_read_header() */
    jpeg_read_header(cinfo, TRUE);

    STEP;

    /* Step 4: set parameters for decompression */
    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */
    cinfoi.out_color_space = JCS_EXT_RGBA;

    STEP;

    /* Step 5: Start decompressor */
    jpeg_start_decompress(cinfo);

    STEP;

    auto surface = createSurface(cinfo->output_width, cinfo->output_height);

    STEP;

    /* We may need to do some setup of our own at this point before readinig
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* Samples per row in output buffer */
    auto row_stride = cinfo->output_width * cinfo->output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    if (cinfo->data_precision == 12)
	buffer12 = (J12SAMPARRAY)(*cinfo->mem->alloc_sarray)
	    ((j_common_ptr)cinfo, JPOOL_IMAGE, row_stride, 1);
    else
	buffer = (*cinfo->mem->alloc_sarray)
	    ((j_common_ptr)cinfo, JPOOL_IMAGE, row_stride, 1);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    STEP;

    auto interval = cinfo->output_height > 1000 ? cinfo->output_height / 20 : 0;
    std::size_t t = interval;

    STEP;

    /* Here we use the library's state variable cinfo->output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    if (cinfo->data_precision == 12) {
        STEP;
	while (cinfo->output_scanline < cinfo->output_height) {
	    /* jpeg12_read_scanlines expects an array of pointers to scanlines.
	     * Here the array is only one element long, but you could ask for
	     * more than one scanline at a time if that's more convenient.
	     */
	    (void)jpeg12_read_scanlines(cinfo, buffer12, 1);
	    // if (*(char *)&little_endian == 1)
	    {
		/* Swap MSB and LSB in each sample */
		for (int col = 0; col < row_stride; col++)
		    buffer12[0][col] = ((buffer12[0][col] & 0xFF) << 8) |
			((buffer12[0][col] >> 8) & 0xFF);
	    }
	    Surface_write(surface, 0, cinfo->output_scanline - 1, cinfo->output_width, 1, (Color*)buffer);
	    // fwrite(buffer12[0], 1, row_stride * sizeof(J12SAMPLE), outfile);
	}
    } else {
        STEP;
	while (cinfo->output_scanline < cinfo->output_height) {
	    /* jpeg_read_scanlines expects an array of pointers to scanlines.
	     * Here the array is only one element long, but you could ask for
	     * more than one scanline at a time if that's more convenient.
	     */
	    if (interval > 1 && (t++ >= interval)) {
		log("decoding {}%", cinfo->output_scanline * 100 / cinfo->output_height);
		t = 0;
	    }
	    (void)jpeg_read_scanlines(cinfo, buffer, 1);
	    Surface_write(surface, 0, cinfo->output_scanline - 1, cinfo->output_width, 1, (Color*)buffer[0]);
	}
    }

    STEP;

    /* Step 7: Finish decompression */

    jpeg_finish_decompress(cinfo);

    /* Step 8: Release JPEG decompression object */
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(cinfo);

    fclose(file);
    log("Decoding jpeg {} DONE!", name);
    return surface;
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
	printf("parseobj error: expected 2 arguments, got %d.\n", argc);
	return 1;
    }
    auto ok = readImageJPG(argv[0]);
    message("{} {:#x} {:#x} surface", argv[1], getpid(), ok);
    return 0;
}
