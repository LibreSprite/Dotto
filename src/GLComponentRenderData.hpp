#pragma once

#include "GLShader.hpp"
#include "Log.hpp"
#include "MeshAttribute.hpp"
#include "Model.hpp"
#include "Node.hpp"
#include "String.hpp"
#include "Uniform.hpp"
#include "Vector.hpp"
#include "Surface.hpp"
#include "ShaderBuilder.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class GLTexture {
public:
    GLuint id{};

    GLTexture() {
        glGenTextures(1, &id);
    }

    ~GLTexture() {
	if (id) {
	    glDeleteTextures(1, &id);
	}
    }
};

inline uint32_t glType(MeshAttribute::ElementType type) {
    switch (type) {
    case MeshAttribute::ElementType::Float: return GL_FLOAT;
    case MeshAttribute::ElementType::Int32: return GL_INT;
    case MeshAttribute::ElementType::Int16: return GL_SHORT;
    case MeshAttribute::ElementType::Int8:  return GL_BYTE;
    }
    return GL_FLOAT;
}

class UniformUploader {
    template<typename F, typename Ret, typename A, typename B> static A helper(Ret (F::*)(A, B));
    template<typename F, typename Ret, typename A, typename B> static A helper(Ret (F::*)(A, B) const);
    template<typename F> struct first_argument { typedef decltype( helper(&F::operator()) ) type;};

public:
    using Uploader = void (*)(const void*, GLint uniformLocation);
    static inline std::unordered_map<std::size_t, Uploader> uploaders;

    static Uploader find(const UniformRef& ref) {
        auto it = uploaders.find(ref.type.hash_code());
        if (it != uploaders.end())
            return it->second;
        LOG("No uploader for ", ref.type.name());
        return [](const void*, GLint){};
    }

    template <typename Func>
    UniformUploader(Func&& f) {
        using Type = std::remove_reference_t<std::remove_cv_t<typename first_argument<Func>::type>>;
        static Func s{std::forward<Func>(f)};
        uploaders[typeid(Type).hash_code()] = [](const void* v, GLint uniformLocation) {
            s(*reinterpret_cast<const Type*>(v), uniformLocation);
        };
    }
};

inline UniformUploader u_float{[](float f, GLint uniformLocation) {
    glUniform1f(uniformLocation, f);
    GFXLOG("glUniform1f", uniformLocation, f);
}};

inline UniformUploader u_UV{[](const UV& f, GLint uniformLocation) {
    glUniform2f(uniformLocation, f.x, f.y);
    GFXLOG("glUniform2f", uniformLocation, f.x, f.y);
}};

inline UniformUploader u_Vector{[](const Vector& f, GLint uniformLocation) {
    glUniform3f(uniformLocation, f.x, f.y, f.z);
    GFXLOG("glUniform3f", uniformLocation, f.x, f.y, f.z);
}};

inline UniformUploader u_RGBA{[](const RGBA& f, GLint uniformLocation) {
    glUniform4f(uniformLocation, f.r, f.g, f.b, f.a);
    GFXLOG("glUniform4f", uniformLocation, f.r, f.g, f.b, f.a);
}};

inline UniformUploader u_Matrix{[](const Matrix& f, GLint uniformLocation) {
    glUniformMatrix4fv(uniformLocation, 1, GL_TRUE, f.v);
    GFXLOG("glUniformMatrix4fv", uniformLocation, 1, GL_TRUE);
    GFXLOG(f.v[0], f.v[1], f.v[2], f.v[3]);
    GFXLOG(f.v[4], f.v[5], f.v[6], f.v[7]);
    GFXLOG(f.v[8], f.v[9], f.v[10], f.v[11]);
    GFXLOG(f.v[12], f.v[13], f.v[14], f.v[15]);
}};

inline uint32_t nextTextureUnit{};
inline UniformUploader u_Surface{[](const std::shared_ptr<Surface>& surface, GLint uniformLocation) {
    if (!surface) {
	return;
    }
    if (!surface->texture) {
	auto texture = std::make_shared<GLTexture>();
	surface->texture = texture;
	surface->dirty.expand({0, 0, surface->width, surface->height});
    }
    // auto texture = std::static_pointer_cast<GLTexture>(surface->texture);
    auto texture = static_cast<GLTexture*>(surface->texture.get());
    glActiveTexture(GL_TEXTURE0 + nextTextureUnit);
    GFXLOG("glActiveTexture", nextTextureUnit);
    GLCHECK_MSG(nextTextureUnit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    GFXLOG("glBindTexture", GL_TEXTURE_2D, texture->id);
    GLCHECK;
    if (!surface->dirty.empty()) {
	surface->dirty.clear();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        GFXLOG("glTexParameteri");
        GLCHECK;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GFXLOG("glTexParameteri");
        GLCHECK;
        // TODO: Use dirtyRegion to upload only what changed
	glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     surface->width,
                     surface->height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     surface->pixels.data());
        GFXLOG("glTexParameteri");
        GLCHECK;
    }
    glUniform1i(uniformLocation, nextTextureUnit);
    GFXLOG("glUniform1i", uniformLocation, nextTextureUnit);
    GLCHECK;
    nextTextureUnit++;
}};

class ComponentRenderData {
public:
    static inline std::unordered_map<std::string, std::shared_ptr<GLShader>> shaderCache;
    static inline ShaderBuilder builder;

    float priority{};
    Matrix transform;
    std::vector<uint8_t> raw;
    std::vector<uint32_t>* elements{};
    std::shared_ptr<GLShader> shader;
    GLuint vbo{};
    GLuint vao{};
    GLuint veo{};
    std::size_t stride{};
    std::size_t length{};

    struct AttrDef {
        std::string name;
        int32_t index{};
        std::size_t offset{};
        std::size_t elementCount{};
        MeshAttribute::ElementType type;
    };
    std::vector<AttrDef> attributes;

    struct UniformDef {
        std::size_t index{};
        std::shared_ptr<UniformRef> ref;
        UniformUploader::Uploader uploader;
    };
    std::vector<UniformDef> uniforms;

    void resize(Mesh& mesh) {
        length = 0;
        stride = 0;
        for (auto& entry : mesh.attributes) {
            auto attr = entry.second.get();
            stride += attr->elementSize();
            length = std::max(length, attr->length());
        }
        raw.resize(length * stride);
    }

    void rebuildGeometry(RenderableNode::Component& component) {
        if (!component.mesh || !component.material)
            return;
        auto& mesh = *component.mesh;
        auto& mat = *component.material;
	elements = mesh.elements.empty() ? nullptr : &mesh.elements;
        resize(mesh);
        std::size_t offset = 0;
        for (auto& entry : mesh.attributes) {
            auto attr = entry.second.get();
	    attr->dirty = false;
            attr->write({raw, offset, stride});
            auto size = attr->elementSize();
            attributes.push_back({
                    entry.first,
                    -1,
                    offset,
                    attr->elementCount(),
                    attr->type()
                });
            offset += size;
        }
    }

    void rebuildMaterial(RenderableNode::Component& component) {
	std::string acc;
	for (auto& entry : component.material->tags)
	    acc += entry + " ";
	if (auto it = shaderCache.find(acc); it != shaderCache.end()) {
	    shader = it->second;
	} else {
	    auto vert = builder.processShaderSource("vertex", component.material->tags);
            GLCHECK;
	    auto frag = builder.processShaderSource("fragment", component.material->tags);
            GLCHECK;
	    shader = GLShader::create(vert, frag);
            GLCHECK;
	    if (shader) {
		shaderCache[acc] = shader;
	    }
	}
    }

    void rebind(RenderableNode& node, RenderableNode::Component& component) {
        if (!vbo)
            glGenBuffers(1, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, raw.size(), raw.data(), GL_STATIC_DRAW);

        if (!vao)
            glGenVertexArrays(1, &vao);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        for (auto& attrib : attributes) {
            auto it = shader->attributes.find(attrib.name);
            if (it == shader->attributes.end()) {
                attrib.index = -1;
            } else {
                attrib.index = it->second.index;
                glVertexAttribPointer(attrib.index, attrib.elementCount, glType(attrib.type), GL_FALSE, stride, (void*)attrib.offset);
            }
        }

        if (!component.mesh->elements.empty()) {
            glGenBuffers(1, &veo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements->size() * 4, elements->data(), GL_STREAM_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        uniforms.clear();

        auto sceneUniforms = Scene::main->uniforms.read();
        auto nodeUniforms = node.uniforms.read();
        auto materialUniforms = component.material->uniforms.read();

        for (auto& entry : shader->uniforms) {
            auto& name = entry.first;

            auto it = nodeUniforms->find(name);
            if (it == nodeUniforms->end()) {
                it = materialUniforms->find(name);
                if (it == materialUniforms->end()) {
                    it = sceneUniforms->find(name);
                    if (it == sceneUniforms->end())
                        continue;
                }
            }

            auto ref = it->second;
            if (!ref)
                continue;

            auto uploader = UniformUploader::find(*it->second);

            uniforms.push_back({
                    entry.second.index,
                    ref,
                    uploader
                });
        }
    }

    bool update(RenderableNode& node, RenderableNode::Component& component) {
        if (!component.mesh || !component.material)
            return false;

        bool needsRebind = false;
        if (raw.empty() || component.mesh->dirty()) {
            rebuildGeometry(component);
            needsRebind = true;
        }

        if (!shader) {
            rebuildMaterial(component);
            if (!shader) {
                component.material.reset();
                return false;
            }
            needsRebind = true;
        }

        if (needsRebind || component.material->dirty) {
	    component.material->dirty = false;
            rebind(node, component);
        }

        return true;
    }

    void draw() {
        if (!shader)
            return;

        shader->use();
        GLCHECK;
        glBindVertexArray(vao);
        GLCHECK;

	nextTextureUnit = 0;

        for (auto& uniform : uniforms) {
            uniform.uploader(uniform.ref->raw(), uniform.index);
            GLCHECK_MSG(shader->uniformNameFromIndex(uniform.index));
        }
        GLCHECK;

        for (auto& attrib : attributes) {
            if (attrib.index > -1)
                glEnableVertexAttribArray(attrib.index);
        }
        GLCHECK;

	if (elements) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
            GLCHECK;
            glDrawElements(GL_TRIANGLES, elements->size(),  GL_UNSIGNED_INT, 0);
            GLCHECK;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
	    glDrawArrays(GL_TRIANGLES, 0, length);
            GLCHECK;
	}

        for (auto& attrib : attributes) {
            if (attrib.index > -1)
                glDisableVertexAttribArray(attrib.index);
        }
        GLCHECK;

    }
};
