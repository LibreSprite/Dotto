#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "Index.hpp"
#include "MeshAttribute.hpp"

class Mesh : public std::enable_shared_from_this<Mesh> {
public:
    AutoIndex key{this};

    std::shared_ptr<void> rendererData;

    template<typename Type>
    Attribute<Type>& addAttribute(const std::string& name) {
        auto attr = new Attribute<Type>();
        attributes[name].reset(attr);
        return *attr;
    }

    bool dirty() {
	for (auto& attr : attributes) {
	    if (attr.second->dirty)
		return true;
	}
	return false;
    }

    std::unordered_map<std::string, std::unique_ptr<MeshAttribute>> attributes;
    std::vector<uint32_t> elements;
};
