#include "Surface.hpp"
#include "VM.hpp"
#include "Log.hpp"
#include <algorithm>
#include <cstdint>
#include <mutex>

std::shared_ptr<Surface> Surface::create() {
    auto ptr = std::make_shared<Surface>();
    return ptr->key.init(ptr);
}

void Surface::resize(uint32_t width, uint32_t height) {
    if (width == this->width && height == this->height)
	return;
    std::unique_lock lock{mutex};
    pixels.resize(width * height);
    this->width = width;
    this->height = height;
}

void Surface::fill(Color color) {
    std::shared_lock lock{mutex};
    std::fill(pixels.begin(), pixels.end(), color);
    dirty = {0, 0, width, height};
}

void Surface::write(Rect region, Color* data) {
    std::shared_lock lock{mutex};
    if (region.x >= width || region.y >= height || !data)
	return;
    if (static_cast<int32_t>(region.x + region.width) <= 0 ||
	static_cast<int32_t>(region.y + region.height) <= 0)
	return;

    // LOG("Writing to region ", region.x, " ", region.y, " ", region.width, " ", region.height);

    uint32_t maxX = region.width;
    uint32_t minX = 0;
    uint32_t maxY = region.height;
    uint32_t minY = 0;
    uint32_t stride = region.width;

    if (region.x < 0) {
	region.width += region.x;
	minX = -region.x;
	region.x = 0;
    }

    if (region.y < 0) {
	region.height += region.y;
	minY = -region.y;
	region.y = 0;
    }

    if (region.x + region.width > width) {
	maxX = region.width = width - region.x;
    }

    if (region.y + region.height > height) {
	maxY = region.height = height - region.y;
    }

    dirty.expand(region);

    for (uint32_t readY = minY; readY < maxY; ++readY) {
	for (uint32_t readX = minX; readX < maxX; ++readX) {
	    pixels[(readY + region.y) * width + region.x + readX].rgba = data[readY * stride + readX].rgba;
	}
    }
}

std::shared_ptr<Surface> Surface::find(uint32_t surfaceId) {
    auto surface = Index<std::shared_ptr<Surface>>::find(surfaceId);
    if (!surface) {
	LOG("Invalid surface id ", std::hex, surfaceId, std::dec);
    }
    return surface ? *surface : std::shared_ptr<Surface>{};
}

static void createSurface(const VM::Args& args) {
    auto surface = Surface::create();
    surface->resize(args.get(0), args.get(1));
    args.result = *surface->key;
}

static void Surface_resize(const VM::Args& args) {
    if (auto surface = Surface::find(args.get(0))) {
	surface->resize(args.get(1), args.get(2));
    }
}

static void Surface_fill(const VM::Args& args) {
    if (auto surface = Surface::find(args.get(0))) {
	surface->fill({
		static_cast<uint8_t>(args.get(1)),
		    static_cast<uint8_t>(args.get(2)),
		    static_cast<uint8_t>(args.get(3)),
		    static_cast<uint8_t>(args.get(4))
	    });
    }
}

static void Surface_write(const VM::Args& args) {
    if (auto surface = Surface::find(args.get(0))) {
	surface->write(Rect{
		args.get<int32_t>(1),
		args.get<int32_t>(2),
		args.get<uint32_t>(3),
		args.get<uint32_t>(4)
	    }, reinterpret_cast<Color*>(args.getPtr(5)));
    }
}

static void Surface_width(const VM::Args& args) {
    if (auto surface = Surface::find(args.get(0))) {
	args.result = surface->width;
    } else {
	args.result = 0;
    }
}

static void Surface_height(const VM::Args& args) {
    if (auto surface = Surface::find(args.get(0))) {
	args.result = surface->height;
    } else {
	args.result = 0;
    }
}

static VM::API api {{
        {"createSurface", createSurface},
	{"Surface_resize", Surface_resize},
	{"Surface_fill", Surface_fill},
	{"Surface_write", Surface_write},
	{"Surface_width", Surface_width},
	{"Surface_height", Surface_height},
    }};
