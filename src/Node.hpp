#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Index.hpp"
#include "Uniform.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

class RenderableNode;

class Node : public std::enable_shared_from_this<Node> {
    static inline std::unordered_map<std::string, Node*> nameIndex;

    std::string name;

    void unindex() {
        if (!name.empty()) {
            auto it = nameIndex.find(name);
            if (it != nameIndex.end() && it->second == this)
                nameIndex.erase(it);
        }
    }

    void index() {
        if (!name.empty())
            nameIndex[name] = this;
    }

public:
    AutoIndex key{this};

    Node() = default;

    Node(const std::string& name) : name{name} {index();}

    virtual ~Node() {unindex();}

    void rename(const std::string& newName) {
        if (newName == name)
            return;
        unindex();
        name = newName;
        index();
    }

    virtual RenderableNode* renderable() {return nullptr;}
    std::vector<std::shared_ptr<Node>> children;

    Vector position;
    Matrix rotation;
    Vector scale;

    Matrix transform;
};

class RenderableNode : public Node {
public:
    RenderableNode() = default;
    RenderableNode(const std::string& name) : Node{name} {}

    RenderableNode* renderable() override {return this;}

    std::unordered_map<std::string, std::shared_ptr<UniformRef>> uniforms = {
        {"transform", std::make_shared<UniformRef>(transform)}
    };

    struct Component {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
        std::shared_ptr<void> rendererData;
    };

    std::vector<Component> components;
};

class Scene {
public:
    std::shared_ptr<Node> root = std::make_shared<Node>();
    std::shared_ptr<Node> camera = std::make_shared<Node>();

    void resize(int width, int height) {
        this->width = width;
        this->height = height;
        projection = Matrix::projection(width, height, near, far, FOV * (3.14159265358979323f / 180.0f));
    }

    int width = 100;
    int height = 100;
    float near = 1.0f;
    float far = 1000.0f;
    float FOV = 60.0f;
    Matrix projection;
    std::unordered_map<std::string, std::shared_ptr<UniformRef>> uniforms = {
        {"projection", std::make_shared<UniformRef>(projection)}
    };

    static inline Scene* main{};
};
