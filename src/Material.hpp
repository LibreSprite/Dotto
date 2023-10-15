#pragma once

#include "Index.hpp"
#include "Uniform.hpp"
#include <string>
#include <memory>
#include <set>

class Material : public std::enable_shared_from_this<Material> {
public:
    AutoIndex key{this};
    bool isTransparent = false;
    bool dirty = true;
    std::unordered_map<std::string, std::shared_ptr<UniformRef>> uniforms;
    std::set<std::string> tags;
};
