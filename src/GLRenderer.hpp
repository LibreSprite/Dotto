#pragma once

#include "Matrix.hpp"
#include "Node.hpp"
#include "Events.hpp"

#include <memory>
#include <string>
#include <vector>


class ComponentRenderData;

class GLRenderer {
    void enqueueComponents(const Matrix&, Node&, Scene*);
    void drawComponents();
    std::vector<ComponentRenderData*> queue;

public:
    void init(uint32_t major, uint32_t minor, const std::string profile);
    void shutdown();
    void draw(Scene*);
};
