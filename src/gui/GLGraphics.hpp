// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <GL/gl.h>
#include <GLES3/gl3.h>

#include <common/match.hpp>
#include <common/Rect.hpp>
#include <common/Surface.hpp>
#include <gui/Graphics.hpp>
#include <gui/Texture.hpp>
#include <log/Log.hpp>

class GLTexture : public Texture {
public:
    U32 id = 0;
    U32 width = 0;
    U32 height = 0;
    F32 iwidth = 0;
    F32 iheight = 0;
    bool dirty = true;

    GLTexture() {
        glGenTextures(1, &id);
    }

    ~GLTexture() {
        if (id)
            glDeleteTextures(1, &id);
    }

    void bind(U32 target) {
        glBindTexture(target, id);
    }
};

class GLGraphics;

class GLTextureInfo : public TextureInfo {
public:
    ~GLTextureInfo();

    std::shared_ptr<GLTexture> get(const std::shared_ptr<GLGraphics>& context);

    void setDirty() {
        for (auto& entry : textures) {
            entry.second->dirty = true;
        }
    }

    HashMap<GLGraphics*, std::shared_ptr<GLTexture>> textures;
};

class GLGraphics : public Graphics, public std::enable_shared_from_this<GLGraphics> {
public:
    HashSet<GLTextureInfo*> textureInfo;
    U32 VBO = 0;
    U32 VAO = 0;
    U32 shader = 0;
    Vector<F32> vertices;
    std::shared_ptr<GLTexture> activeTexture;
    F32 iwidth, iheight;
    S32 width, height;

    void init() {
        glGenBuffers(1, &VBO);
        glGenVertexArrays(1, &VAO);

        auto vertexShader = compile(GL_VERTEX_SHADER,
            "#version 330 core\n"
            "in vec3 position;\n"
            "in vec2 srcUV;\n"
            "out vec2 uv;\n"
            "void main() {\n"
            "   gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
            "   uv = srcUV;\n"
            "}");

        auto fragmentShader = compile(GL_FRAGMENT_SHADER,
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec2 uv;\n"
            "uniform sampler2D surface;\n"
            "void main() {\n"
            "    FragColor = texture(surface, uv);\n"
            "}");

        shader = link(vertexShader, fragmentShader);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    U32 compile(U32 type, const char* source) {
        U32 shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        int  success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glDeleteShader(shader);
            shader = 0;
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            logE("ERROR::SHADER::COMPILATION_FAILED ", type, " ", infoLog);
        }

        return shader;
    }

    U32 link(U32 vertexShader, U32 fragmentShader) {
        U32 program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glDeleteProgram(program);
            program = 0;
            char infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            logE("ERROR::SHADER::LINK_FAILED ", infoLog);
        }
        return program;
    }

    ~GLGraphics() {
        while (!textureInfo.empty()) {
            auto it = textureInfo.begin();
            (*it)->textures.erase(this);
            textureInfo.erase(it);
        }

        if (VBO)
            glDeleteBuffers(1, &VBO);
        if (VAO)
            glDeleteVertexArrays(1, &VAO);
        if (shader)
            glDeleteProgram(shader);
    }

    void begin(Rect& globalRect, Color& clearColor) {
        clip = globalRect;
        width = globalRect.width;
        iwidth = 2.0f / width;
        height = globalRect.height;
        iheight = 2.0f / height;
        glViewport(0, 0, width, height);
        glClearColor(clearColor.r/255.0f,
                     clearColor.g/255.0f,
                     clearColor.b/255.0f,
                     clearColor.a/255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void end() {
        flush();
    }

    U32 currentBufferSize = 0;
    U32 currentShader = 0;

    void flush() {
        if (vertices.empty())
            return;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        U32 requiredSize = vertices.size() * sizeof(vertices[0]);
        if (requiredSize > currentBufferSize) {
            glBufferData(GL_ARRAY_BUFFER, requiredSize, vertices.data(), GL_STREAM_DRAW);
            currentBufferSize = requiredSize;
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, requiredSize, vertices.data());
        }

        if (currentShader != shader) {
            currentShader = shader;
            glUseProgram(shader);
        }

        activeTexture->bind(GL_TEXTURE_2D);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 5);

        vertices.clear();
        activeTexture.reset();
    }

    void upload(Surface& surface, GLTexture* texture) {
        texture->dirty = false;
        texture->bind(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface.width(), surface.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, surface.data());
        texture->width = surface.width();
        texture->iwidth = 1.0f / texture->width;
        texture->height = surface.height();
        texture->iheight = 1.0f / texture->height;
        // glGenerateMipmap(GL_TEXTURE_2D);
    }

    struct Vertex {
        F32 x;
        F32 y;
        F32 z;
        F32 u;
        F32 v;
    };

    void push(const Vertex& vtx) {
        constexpr const F32 depthFactor = 0.0000001f;
        vertices.push_back(vtx.x * iwidth - 1.0f);
        vertices.push_back(1.0f - vtx.y * iheight);
        vertices.push_back(vtx.z * depthFactor);
        vertices.push_back(vtx.u);
        vertices.push_back(vtx.v);
    }

    struct Rectf {
        F32 x, y, w, h;
        F32 u0, v0, u1, v1;
    };

    void push(F32 z, const Rectf& rect) {
        F32 x1 = rect.x,
            y1 = rect.y,
            x2 = rect.x + rect.w,
            y2 = rect.y + rect.h,
            u0 = rect.u0,
            v0 = rect.v0,
            u1 = rect.u1,
            v1 = rect.v1;

        if (x1 >= clip.right() ||
            x2 <= clip.x ||
            y1 >= clip.bottom() ||
            y2 <= clip.y)
            return;

        if (x1 < clip.x) {
            if (rect.w) {
                u0 -= (u0 - u1) * ((clip.x - x1) / rect.w);
            }
            x1 = clip.x;

        }
        if (x2 > clip.right()) {
            if (rect.w) {
                u1 += (u0 - u1) * ((x2 - clip.right()) / rect.w);
            }
            x2 = clip.right();
        }

        if (y1 < clip.y) {
            if (rect.h) {
                v0 -= (v0 - v1) * ((clip.y - y1) / rect.h);
            }
            y1 = clip.y;
        }
        if (y2 > clip.bottom()) {
            if (rect.h) {
                v1 += (v0 - v1) * ((y2 - clip.bottom()) / rect.h);
            }
            y2 = clip.bottom();
        }

        push({x1, y1, z, u0, v0});
        push({x1, y2, z, u0, v1});
        push({x2, y1, z, u1, v0});
        push({x1, y2, z, u0, v1});
        push({x2, y2, z, u1, v1});
        push({x2, y1, z, u1, v0});
    }

    void push(std::shared_ptr<GLTexture>& texture, const BlitSettings& settings) {
        F32 x = settings.destination.x;
        F32 y = settings.destination.y;
        F32 z = settings.zIndex;
        F32 w = settings.destination.width;
        F32 h = settings.destination.height;

        if (!clip.overlaps(settings.destination))
            return;

        if (activeTexture != texture) {
            flush();
            activeTexture = texture;
        }

        F32 sW = settings.nineSlice.width;
        F32 sH = settings.nineSlice.height;

        if (sW == 0 && settings.nineSlice.x != 0)
            sW = texture->width - settings.nineSlice.x * 2;
        if (sH == 0 && settings.nineSlice.y != 0)
            sH = texture->height - settings.nineSlice.y * 2;

        if (sW <= 0 || sH <= 0) {
            push(z, {x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f});
        } else {
            F32 sX = settings.nineSlice.x;
            F32 sY = settings.nineSlice.y;
            F32 nsX = sX * texture->iwidth;
            F32 nsY = sY * texture->iheight;
            F32 nsW = sW * texture->iwidth;
            F32 nsH = sH * texture->iheight;
            F32 rW = texture->width - sW - sX;
            F32 rH = texture->height - sH - sY;

            push(z, {x, y, sX, sY, 0.0f, 0.0f, nsX, nsY});
            push(z, {x + sX, y, w - sX - rW, sY, nsX, 0.0f, nsX + nsW, nsY});
            push(z, {x + w - rW, y, rW, sY, nsX + nsW, 0.0f, 1.0f, nsY});

            y += sY;
            push(z, {x, y, sX, h - sY - rH, 0.0f, nsY, nsX, nsY + nsH});
            push(z, {x + sX, y, w - sX - rW, h - sY - rH, nsX, nsY, nsX + nsW, nsY + nsH});
            push(z, {x + w - rW, y, rW, h - sY - rH, nsX + nsW, nsY, 1.0f, nsY + nsH});

            y += h - sY - rH;
            push(z, {x, y, sX, rH, 0.0f, nsY + nsH, nsX, 1.0f});
            push(z, {x + sX, y, w - sX - rW, rH, nsX, nsY + nsH, nsX + nsW, 1.0f});
            push(z, {x + w - rW, y, rW, rH, nsX + nsW, nsY + nsH, 1.0f, 1.0f});
        }
    }

    void blit(const BlitSettings& settings) override {
        auto& surface = *settings.surface;
        if (!surface.textureInfo)
            surface.textureInfo = std::make_shared<GLTextureInfo>();
        auto texture = std::static_pointer_cast<GLTextureInfo>(surface.textureInfo)->get(shared_from_this());

        if (texture->dirty)
            upload(surface, texture.get());

        push(texture, settings);
    }

    Rect pushClipRect(const Rect& rect) override {
        auto copy = clip;
        flush();
        clip.intersect(rect);
        return copy;
    }

    void setClipRect(const Rect& rect) override {
        flush();
        clip = rect;
    }
};

inline std::shared_ptr<GLTexture> GLTextureInfo::get(const std::shared_ptr<GLGraphics>& context) {
    auto it = textures.find(context.get());
    if (it == textures.end()) {
        auto texture = std::make_shared<GLTexture>();
        it = textures.emplace(context.get(), texture).first;
        context->textureInfo.insert(this);
    }
    return it->second;
}

inline GLTextureInfo::~GLTextureInfo() {
    for (auto& entry : textures) {
        entry.first->textureInfo.erase(this);
    }
}
