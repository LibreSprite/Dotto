#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Index.hpp"
#include "Log.hpp"
#include "Uniform.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

class RenderableNode;

class Node : public AutoIndex<Node> {
    // static inline std::unordered_map<std::string, Node*> nameIndex;

    std::string name;

    void unindex() {
        // if (!name.empty()) {
        //     auto it = nameIndex.find(name);
        //     if (it != nameIndex.end() && it->second == this)
        //         nameIndex.erase(it);
        // }
    }

    void index() {
        // if (!name.empty())
        //     nameIndex[name] = this;
    }

protected:
    Node(const std::string& name = "") : name{name} {index();}

public:
    virtual ~Node() {unindex();}

    void rename(const std::string& newName) {
        if (newName == name)
            return;
        unindex();
        name = newName;
        index();
    }

    virtual RenderableNode* renderable() {return nullptr;}
    Shared<std::vector<std::shared_ptr<Node>>> children;

    bool visible {true};
    Vector position;
    Matrix rotation;
    Vector scale {1, 1, 1};
    Matrix transform;
};

class RenderableNode : public Node {
public:
    RenderableNode(const std::string& name = "") : Node{name} {
        uniforms.write([&](auto& uniforms){
            uniforms = {
                {"transform", std::make_shared<UniformRef>(transform)}
            };
        });
    }

    RenderableNode* renderable() override {return this;}

    Shared<std::unordered_map<std::string, std::shared_ptr<UniformRef>>> uniforms;

    struct Component {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
        std::shared_ptr<void> rendererData;
    };

    Shared<std::vector<Component>> components;
};

class Scene {
public:
    std::shared_ptr<Node> root = Node::create();
    std::shared_ptr<Node> camera = Node::create();

    void resize(int width, int height) {
        this->width = width;
        this->height = height;
        GFXLOG("Scene::resize", width, height);
        projection = Matrix::projection(width, height, near, far, FOV * (3.14159265358979323f / 180.0f));
    }

    int width = 100;
    int height = 100;
    float near = 1.0f;
    float far = 1000.0f;
    float FOV = 60.0f;
    Matrix projection;
    Shared<std::unordered_map<std::string, std::shared_ptr<UniformRef>>> uniforms;

    Scene() {
        uniforms.write([&](auto& uniforms) {
            uniforms = {
                {"projection", std::make_shared<UniformRef>(projection)}
            };
        });
    }

    static inline Scene* main{};
};
