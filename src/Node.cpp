#include "MainThread.hpp"
#include "Material.hpp"
#include "Matrix.hpp"
#include "Node.hpp"
#include "VM.hpp"
#include <cstdint>

static RenderableNode* renderableFromNodeId(uint32_t nodeId) {
    auto node = Index<Node*>::find(nodeId);
    if (!node) {
	return 0;
    }
    return (*node)->renderable();
}

static void createRenderable(const VM::Args& args) {
    auto n = std::make_shared<RenderableNode>();

    mainThread([=] {
	if (Scene::main)
	    Scene::main->root->children.push_back(n);
    });

    args.result = *n->key;
}

static void Node_setPosition(const VM::Args& args) {
    auto id = args.get<uint32_t>(0);
    auto x = args.get<float>(1);
    auto y = args.get<float>(2);
    auto z = args.get<float>(3);
    mainThread([=] {
	auto node = Index<Node*>::find(id);
	if (!node)
	    return;
	(*node)->position.set(x, y, z);
    });
}

static void Node_getComponentCount(const VM::Args& args) {
    auto renderable = renderableFromNodeId(args.get<uint32_t>(0));
    if (!renderable) {
	args.result = 0;
	return;
    }
    args.result = static_cast<uint32_t>(renderable->components.size());
}

static void Node_getMesh(const VM::Args& args) {
    auto renderable = renderableFromNodeId(args.get<uint32_t>(0));
    auto meshIndex = args.get<uint32_t>(1);
    if (!renderable || renderable->components.size() <= meshIndex) {
	args.result = 0;
	return;
    }
    auto& mesh = renderable->components[meshIndex].mesh;
    if (!mesh) {
	args.result = 0;
	return;
    }
    args.result = *mesh->key;
}

static void Node_getMaterial(const VM::Args& args) {
    auto renderable = renderableFromNodeId(args.get<uint32_t>(0));
    auto materialIndex = args.get<uint32_t>(1);
    if (!renderable || renderable->components.size() <= materialIndex) {
	args.result = 0;
	return;
    }
    auto& material = renderable->components[materialIndex].material;
    if (!material) {
	args.result = 0;
	return;
    }
    args.result = *material->key;
}

static void Node_addComponent(const VM::Args& args) {
    auto renderableId = args.get<uint32_t>(0);
    auto meshId = args.get<uint32_t>(1);
    auto materialId = args.get<uint32_t>(2);

    mainThread([=] {
	auto renderable = renderableFromNodeId(renderableId);
	auto mesh = Index<Mesh*>::find(meshId);
	auto material = Index<Material*>::find(materialId);
	if (!renderable || !mesh || !material) {
	    LOG("Could not add component to node. ", !!renderable, mesh.has_value(), material.has_value());
	    return;
	}

	renderable->components.push_back({
		.mesh = (*mesh)->shared_from_this(),
		.material = (*material)->shared_from_this()
	    });
    });
}

static void Node_rotate(const VM::Args& args) {
    auto nodeId = args.get<uint32_t>(0);
    auto angle = args.get<float>(1);
    Vector3 axis {
	args.get<float>(2),
	args.get<float>(3),
	args.get<float>(4)
    };
    auto mat = Matrix::rotation(angle, axis);
    mainThread([=] {
	auto node = Index<Node*>::find(nodeId);
	if (!node) {
	    return;
	}
        (*node)->rotation *= mat;
    });
}

static VM::API api {{
	{"createRenderable", createRenderable},
	{"Node_setPosition", Node_setPosition},
	{"Node_rotate", Node_rotate},
	{"Node_getComponentCount", Node_getComponentCount},
	{"Node_getMesh", Node_getMesh},
	{"Node_getMaterial", Node_getMaterial},
	{"Node_addComponent", Node_addComponent},
    }};
