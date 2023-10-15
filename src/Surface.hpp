#pragma once

#include "Index.hpp"
#include "Vector.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <vector>

class Surface {
public:
    AutoIndex key;
    std::shared_mutex mutex;
    std::vector<Color> pixels;
    uint32_t width;
    uint32_t height;
    Rect dirty;
    std::shared_ptr<void> texture;
    void resize(uint32_t width, uint32_t height);
    void fill(Color color);
    void write(Rect region, Color* data);

    static inline uint32_t maxWidth;
    static inline uint32_t maxHeight;
    static std::shared_ptr<Surface> create();
    static std::shared_ptr<Surface> find(uint32_t surfaceId);
};
