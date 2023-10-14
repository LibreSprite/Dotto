#include <cstdint>

#include "Log.hpp"
#include "MainThread.hpp"
#include "Mesh.hpp"
#include "VM.hpp"
#include "Vector.hpp"

static void createMesh(const VM::Args& args) {
    args.result = create<Mesh>();
}

template <typename Type>
static void addAttribute(const VM::Args& args) {
    auto meshId = args.get<uint32_t>(0);
    auto attributeName = args.get<std::string>(1);
    mainThread([=]{
	auto mesh = Index<Mesh*>::find(meshId);
	if (!mesh) {
	    LOG("Could not find mesh ", meshId);
	    return;
	}
	(*mesh)->addAttribute<Type>(attributeName);
    });
}

static void Mesh_clearElements(const VM::Args& args) {
    auto meshId = args.get<uint32_t>(0);
    mainThread([=]{
	auto mesh = Index<Mesh*>::find(meshId);
	if (!mesh) {
	    LOG("Could not find mesh ", meshId);
	    return;
	}
	(*mesh)->elements.clear();
    });
}

static void Mesh_pushElements(const VM::Args& args) {
    auto meshId = args.get<uint32_t>(0);
    auto begin = args.get<uint32_t*>(1);
    auto end = args.get<uint32_t*>(2);
    if (!end || !begin || reinterpret_cast<uintptr_t>(begin) & 3 || reinterpret_cast<uintptr_t>(end) & 3) {
	LOG("pushError: Invalid element data for mesh ", meshId);
	return;
    }
    std::vector<uint8_t> data{begin, end};
    mainThread([=, data=std::move(data)]{
	auto mesh = Index<Mesh*>::find(meshId);
	if (!mesh) {
	    LOG("Could not find mesh ", meshId);
	    return;
	}
	auto& elements = (*mesh)->elements;
	elements.insert(elements.end(), data.begin(), data.end());
    });
}

static void Mesh_pushAttribute(const VM::Args& args) {
    auto meshId = args.get<uint32_t>(0);
    auto attributeName = args.get<std::string>(1);
    auto begin = args.get<uint8_t*>(2);
    auto end = args.get<uint8_t*>(3);
    if (!end || !begin) {
	LOG("pushError: Invalid attribute data for ", attributeName, " in mesh ", meshId);
	return;
    }
    std::vector<uint8_t> data{begin, end};
    mainThread([=, attributeName=std::move(attributeName), data=std::move(data)]{
	auto mesh = Index<Mesh*>::find(meshId);
	if (!mesh) {
	    LOG("Could not find mesh ", meshId);
	    return;
	}
	auto& attributes = (*mesh)->attributes;
	auto it = attributes.find(attributeName);
	if (it == attributes.end()) {
	    LOG("pushError: No attribute ", attributeName, " in mesh ", meshId);
	    return;
	}
	auto& attribute = *it->second;
	attribute.read(data);
    });
}

inline VM::API meshapi {{
        {"createMesh", createMesh},
	{"Mesh_clearElements", Mesh_clearElements},
	{"Mesh_pushElements", Mesh_pushElements},
	{"Mesh_addAttributeFloat",   addAttribute<float>},
	{"Mesh_addAttributeVector2", addAttribute<Vector2>},
	{"Mesh_addAttributeVector3", addAttribute<Vector3>},
	{"Mesh_addAttributeVector4", addAttribute<Vector4>},
	{"Mesh_pushAttribute", Mesh_pushAttribute}
    }};
