#include "API.hpp"
#include <algorithm>
#include <cstdint>
#include "qoi.h"
#include <cstring>
#include <fmt.hpp>
#include <sys/unistd.h>

SurfaceId loadQOIImage(const char* name) {
    qoi_desc desc;
    auto rgba_pixels = reinterpret_cast<std::byte*>(qoi_read(name, &desc, 4));

    /* The first argument is the file to read: */
    if (!rgba_pixels) {
	fprintf(stderr, "Could not parse image: %s\n", name);
	return SurfaceId(0);
    }

    auto surface = createSurface(desc.width, desc.height);
    Surface_write(surface, 0, 0, desc.width, desc.height, reinterpret_cast<Color*>(rgba_pixels));
    free(rgba_pixels);
    return surface;
}


int main(int argc, const char* argv[]) {
    if (argc < 2) {
	printf("parseobj error: expected 2 arguments, got %d.\n", argc);
	return 1;
    }
    auto ok = loadQOIImage(argv[0]);
    message("{} {:#x} {:#x}", argv[1], getpid(), ok);
    return 0;
}
