#include "MainThread.hpp"
#include "Material.hpp"
#include "Matrix.hpp"
#include "Node.hpp"
#include "VM.hpp"
#include <cstdint>

static RenderableNode* renderableFromNodeId(uint32_t nodeId) {
    auto node = Index<Node>::find(nodeId);
    if (!node) {
	return {};
    }
    return node->renderable();
}

static void createNode(const VM::Args& args) {
    auto n = args.create<RenderableNode>();
    args.result = n->key();

    if (Scene::main) {
        Scene::main->root->children.write()->push_back(n);
    }
}

static void Node_setVisible(const VM::Args& args) {
    auto renderable = renderableFromNodeId(args.get<uint32_t>(0));
    if (renderable)
        renderable->visible = args.get<bool>(1);
}

static void Node_getVisible(const VM::Args& args) {
    auto renderable = renderableFromNodeId(args.get<uint32_t>(0));
    args.result = renderable ? renderable->visible : false;
}

static void Node_setPosition(const VM::Args& args) {
    auto id = args.get<uint32_t>(0);
    auto x = args.get<float>(1);
    auto y = args.get<float>(2);
    auto z = args.get<float>(3);
    auto node = Index<Node>::find(id);
    if (node)
	node->position.set(x, y, z);
}

static void Node_setScale(const VM::Args& args) {
    auto id = args.get<uint32_t>(0);
    auto node = Index<Node>::find(id);
    if (!node)
        return;
    auto x = args.get<float>(1);
    auto y = args.get<float>(2);
    auto z = args.get<float>(3);
    node->scale.set(x, y, z);
}

static void Node_getComponentCount(const VM::Args& args) {
    args.result = 0;
    auto renderable = renderableFromNodeId(args.get<uint32_t>(0));
    if (!renderable)
	return;
    args.result = static_cast<uint32_t>(renderable->components.read()->size());
}

static void Node_getMesh(const VM::Args& args) {
    args.result = 0;
    auto renderable = renderableFromNodeId(args.get<uint32_t>(0));
    if (!renderable)
	return;
    auto meshIndex = args.get<uint32_t>(1);
    renderable->components.read([&](auto& components){
        if (components.size() <= meshIndex)
            return;
        auto& mesh = components[meshIndex].mesh;
        if (!mesh)
            return;
        args.result = mesh->key();
    });
}

static void Node_getMaterial(const VM::Args& args) {
    args.result = 0;
    auto renderable = renderableFromNodeId(args.get<uint32_t>(0));
    if (!renderable)
	return;
    auto materialIndex = args.get<uint32_t>(1);
    renderable->components.read([&](auto& components){
        if (components.size() <= materialIndex)
            return;
        auto& material = components[materialIndex].material;
        if (!material)
            return;
        args.result = material->key();
    });
}

static void Node_addComponent(const VM::Args& args) {
    auto renderableId = args.get<uint32_t>(0);
    auto renderable = renderableFromNodeId(renderableId);
    if (!renderable) {
        LOG("Could not add component. Invalid Node Id: ", renderableId);
        return;
    }
    auto meshId = args.get<uint32_t>(1);
    auto mesh = Index<Mesh>::find(meshId);
    if (!mesh) {
        LOG("Could not add component. Invalid Mesh Id: ", meshId);
        return;
    }
    auto materialId = args.get<uint32_t>(2);
    auto material = Index<Material>::find(materialId);
    if (!material) {
        LOG("Could not add component. Invalid Material Id: ", materialId);
        return;
    }
    renderable->components.write()->push_back({
            .mesh = mesh,
            .material = material
        });
}

static void Node_rotate(const VM::Args& args) {
    auto nodeId = args.get<uint32_t>(0);
    auto node = Index<Node>::find(nodeId);
    if (!node) {
        return;
    }
    auto angle = args.get<float>(1);
    Vector3 axis {
	args.get<float>(2),
	args.get<float>(3),
	args.get<float>(4)
    };
    node->rotation *= Matrix::rotation(angle, axis);
}

static void Node_setRotation(const VM::Args& args) {
    auto nodeId = args.get<uint32_t>(0);
    auto node = Index<Node>::find(nodeId);
    if (!node) {
        return;
    }
    auto angle = args.get<float>(1);
    Vector3 axis {
	args.get<float>(2),
	args.get<float>(3),
	args.get<float>(4)
    };
    node->rotation = Matrix::rotation(angle, axis);
}

static VM::API api {{
	{"createNode", createNode},
        {"Node_setVisible", Node_setVisible},
        {"Node_getVisible", Node_getVisible},
	{"Node_setPosition", Node_setPosition},
	{"Node_setScale", Node_setScale},
	{"Node_rotate", Node_rotate},
	{"Node_setRotation", Node_setRotation},
	{"Node_getComponentCount", Node_getComponentCount},
	{"Node_getMesh", Node_getMesh},
	{"Node_getMaterial", Node_getMaterial},
	{"Node_addComponent", Node_addComponent},
    }};
