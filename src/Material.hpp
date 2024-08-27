#pragma once

#include "Index.hpp"
#include "Uniform.hpp"
#include <shared_mutex>
#include <string>
#include <memory>
#include <set>

class Material : public AutoIndex<Material> {
protected:
    Material() = default;
public:
    bool isTransparent = false;
    bool dirty = true;
    Shared<std::unordered_map<std::string, std::shared_ptr<UniformRef>>> uniforms;
    std::set<std::string> tags;
};
