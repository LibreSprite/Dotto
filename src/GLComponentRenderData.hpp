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

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
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
}};

inline UniformUploader u_UV{[](const UV& f, GLint uniformLocation) {
    glUniform2f(uniformLocation, f.x, f.y);
}};

inline UniformUploader u_Vector{[](const Vector& f, GLint uniformLocation) {
    glUniform3f(uniformLocation, f.x, f.y, f.z);
}};

inline UniformUploader u_RGBA{[](const RGBA& f, GLint uniformLocation) {
    glUniform4f(uniformLocation, f.r, f.g, f.b, f.a);
}};

inline UniformUploader u_Matrix{[](const Matrix& f, GLint uniformLocation) {
    glUniformMatrix4fv(uniformLocation, 1, GL_TRUE, f.v);
}};

inline uint32_t textureUnit{};
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
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    if (!surface->dirty.empty()) {
	surface->dirty.clear();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
    }
    glUniform1i(uniformLocation, textureUnit);
    textureUnit++;
}};

class ComponentRenderData {
public:
    static inline std::unordered_map<std::string, std::shared_ptr<GLShader>> shaderCache;

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

    std::pair<std::string, std::string> findShaderSegment(const std::string& name) {
	using namespace std;
	auto major = std::to_string(GLShader::major);
	auto minor = std::to_string(GLShader::minor);
	auto full = "material." + GLShader::profile + major + minor + "0" + "." + name;
	auto segment = getShaderSegment(full);
	if (!segment.empty())
	    return {full, segment};
	full = "material." + GLShader::profile + major + "." + name;
	segment = getShaderSegment(full);
	if (!segment.empty())
	    return {full, segment};
	full = "material." + GLShader::profile + "." + name;
	segment = getShaderSegment(full);
	if (!segment.empty())
	    return {full, segment};
	full = "material.opengl." + name;
	segment = getShaderSegment(full);
	return {full, segment};
    }

    std::string getShaderSegment(const std::string tag) {
	return Model::root.get(tag, "");
    }

    std::string processShaderSource(const std::string type, const std::set<std::string>& tags) {
	using Tag = std::pair<std::string, std::string>;
	std::vector<Tag> parts;
	auto activeTag = ~std::size_t{};

	std::vector<std::string> tagQueue {type};
	for (auto& tag : tags)
	    tagQueue.push_back(tag + "." + type);

	for (auto& tag : tagQueue) {
	    auto [fullPath, src] = findShaderSegment(tag);
	    auto tagSrc = split(src, "\n");
	    for (auto& line : tagSrc) {
		auto trimmed = trim(line);
		if (!trimmed.empty() && trimmed.back() == '>') {
		    if (trimmed.find("//section<") == 0) {
			auto sectionName = trimmed.substr(10, trimmed.size() - 11);
			std::size_t targetTag = 0;
			auto max = parts.size();
			for (; targetTag < max; ++targetTag) {
			    if (parts[targetTag].first == sectionName)
				break;
			}
			if (targetTag >= max) {
			    targetTag = activeTag + 1;
			    auto it = parts.begin();
			    std::advance(it, targetTag);
			    parts.insert(it, {sectionName, {}});
			} else {
			    line = "// *** " + fullPath + " ***";
			}
			activeTag = targetTag;
			continue;
		    }
		}
		if (activeTag < parts.size()) {
		    parts[activeTag].second += line + (" // " + fullPath) + "\n";
		} else {
		    LOG("skipping ", activeTag, "[", line, "]");
		}
	    }
	}

	std::string src;
	for (auto& tag : parts) {
	    // src += "// begin " + tag.first + "\n";
	    src += tag.second;
	}
	return src;
    }

    void rebuildMaterial(RenderableNode::Component& component) {
	std::string acc;
	for (auto& entry : component.material->tags)
	    acc += entry + " ";
	if (auto it = shaderCache.find(acc); it != shaderCache.end()) {
	    shader = it->second;
	} else {
	    auto vert = processShaderSource("vertex", component.material->tags);
	    auto frag = processShaderSource("fragment", component.material->tags);
	    shader = GLShader::create(vert, frag);
	    if (shader) {
		shaderCache[acc] = shader;
	    }
	}
    }

    void rebind(RenderableNode& node, RenderableNode::Component& component) {
	textureUnit = 0;
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
        for (auto& entry : shader->uniforms) {
            auto& name = entry.first;

            auto it = node.uniforms.find(name);
            if (it == node.uniforms.end()) {
                it = component.material->uniforms.find(name);
                if (it == component.material->uniforms.end()) {
                    it = Scene::main->uniforms.find(name);
                    if (it == Scene::main->uniforms.end())
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

    void update(RenderableNode& node, RenderableNode::Component& component) {
        if (!component.mesh || !component.material)
            return;

        bool needsRebind = false;
        if (raw.empty() || component.mesh->dirty()) {
            rebuildGeometry(component);
            needsRebind = true;
        }

        if (!shader) {
            rebuildMaterial(component);
            if (!shader) {
                component.material.reset();
                return;
            }
            needsRebind = true;
        }

        if (needsRebind || component.material->dirty) {
	    component.material->dirty = false;
            rebind(node, component);
        }
    }

    void draw() {
        if (!shader)
            return;

        shader->use();
        glBindVertexArray(vao);

        for (auto& uniform : uniforms) {
            uniform.uploader(uniform.ref->raw(), uniform.index);
        }

        for (auto& attrib : attributes) {
            if (attrib.index > -1)
                glEnableVertexAttribArray(attrib.index);
        }

	if (elements) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
            glDrawElements(GL_TRIANGLES, elements->size(),  GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
	    glDrawArrays(GL_TRIANGLES, 0, length);
	}

        for (auto& attrib : attributes) {
            if (attrib.index > -1)
                glDisableVertexAttribArray(attrib.index);
        }

    }
};
