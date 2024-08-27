#include "Index.hpp"
#include "Log.hpp"
#include "MainThread.hpp"
#include "Material.hpp"
#include "String.hpp"
#include "Surface.hpp"
#include "VM.hpp"
#include <cstdint>

static std::shared_ptr<Material> materialFromId(uint32_t id) {
    return Index<Material>::find(id);
}

static void createMaterial(const VM::Args& args) {
    auto tagstr = split(args.get<std::string>(0), " ");
    auto material = args.create<Material>();
    for (auto& tag : tagstr) {
	if (!tag.empty()) {
	    material->tags.insert(tag);
	}
    }
    args.result = material->key();
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
    auto material = materialFromId(materialId);
    if (!material) {
        return;
    }
    auto uniform = args.get<std::string>(1);
    auto textureId = args.get<uint32_t>(2);
    auto texture = Surface::find(textureId);
    if (!texture) {
        LOG("Could not set material texture: invalid texture Id. ", textureId);
	return;
    }
    material->uniforms.write([&](auto& uniforms){
        uniforms[uniform] = std::make_shared<Uniform<std::shared_ptr<Surface>>>(texture);
    });
    material->dirty = true;
}

template <typename Type>
static void setUniform(const VM::Args& args) {
    auto materialId = args.get<uint32_t>(0);
    auto material = materialFromId(materialId);
    if (!material) {
        return;
    }
    auto uniform = args.get<std::string>(1);
    auto value = *args.get<Type*>(2);
    (*material->uniforms.write())[uniform] = std::make_shared<Uniform<Type>>(value);
    material->dirty = true;
}

static VM::API api {{
	{"createMaterial", createMaterial},
	{"Material_addTag", Material_addTag},
	{"Material_setTexture", Material_setTexture},
        {"Material_setUniformFloat", setUniform<float>},
        {"Material_setUniformVector2", setUniform<Vector2>},
        {"Material_setUniformVector3", setUniform<Vector3>},
        {"Material_setUniformVector4", setUniform<Vector4>}
    }};
