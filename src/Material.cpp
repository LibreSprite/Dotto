#include "Index.hpp"
#include "Log.hpp"
#include "MainThread.hpp"
#include "Material.hpp"
#include "String.hpp"
#include "Surface.hpp"
#include "VM.hpp"
#include <cstdint>

static Material* materialFromId(uint32_t id) {
    auto material = Index<Material*>::find(id);
    return material ? *material : nullptr;
}

static void createMaterial(const VM::Args& args) {
    auto tagstr = split(args.get<std::string>(0), " ");
    auto materialId = create<Material>();
    auto material = materialFromId(materialId);
    for (auto& tag : tagstr) {
	if (!tag.empty()) {
	    material->tags.insert(tag);
	}
    }
    args.result = materialId;
}

static void Material_addTag(const VM::Args& args) {
    auto materialId = args.get<uint32_t>(0);
    auto tag = args.get<std::string>(1);
    if (tag.empty())
	return;
    mainThread([=]{
	auto material = materialFromId(materialId);
	if (!material) {
	    return;
	}
	material->tags.insert(tag);
    });
}

static void Material_setTexture(const VM::Args& args) {
    auto materialId = args.get<uint32_t>(0);
    auto uniform = args.get<std::string>(1);
    auto textureId = args.get<uint32_t>(2);
    auto texture = Surface::find(textureId);
    if (!texture) {
	return;
    }
    mainThread([=]{
	auto material = materialFromId(materialId);
	if (!material) {
	    return;
	}
	material->uniforms[uniform] = std::make_shared<Uniform<std::shared_ptr<Surface>>>(texture);
	material->dirty = true;
    });
}

static VM::API api {{
	{"createMaterial", createMaterial},
	{"Material_addTag", Material_addTag},
	{"Material_setTexture", Material_setTexture}
    }};
