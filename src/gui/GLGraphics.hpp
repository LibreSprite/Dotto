// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "log/Log.hpp"
#include <GL/gl.h>
#include <GLES3/gl3.h>

#include <common/match.hpp>
#include <doc/Surface.hpp>
#include <gui/Graphics.hpp>
#include <gui/Rect.hpp>
#include <gui/Texture.hpp>
#include <memory>

class GLTexture : public Texture {
public:
    U32 id = 0;

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

class GLGraphics : public Graphics {
public:
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
            "    FragColor = texture(surface, uv);\n" // vec4(uv.x, uv.y, 0.0f, 1.0f);\n"
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
        if (VBO)
            glDeleteBuffers(1, &VBO);
        if (shader)
            glDeleteProgram(shader);
    }

    void begin(ui::Rect& globalRect, Color& clearColor) {
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

    void flush() {
        if (vertices.empty())
            return;

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glUseProgram(shader);

        glBindVertexArray(VAO);
        activeTexture->bind(GL_TEXTURE_2D);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 5);

        vertices.clear();
        activeTexture.reset();
    }

    void upload(SurfaceRGBA& surface) {
        auto texture = std::static_pointer_cast<GLTexture>(surface.texture);
        surface.clearDirty();
        texture->bind(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface.width(), surface.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, surface.data());
        // glGenerateMipmap(GL_TEXTURE_2D);
    }

    void upload(Surface256& surface) {
        auto texture = std::static_pointer_cast<GLTexture>(surface.texture);
        surface.clearDirty();
    }

    void push(std::shared_ptr<GLTexture>& texture, const BlitSettings& settings) {
        auto x = settings.destination.x;
        auto y = settings.destination.y;
        auto z = settings.zIndex;
        auto w = settings.destination.width;
        auto h = settings.destination.height;

        constexpr const F32 depthFactor = 0.0000001f;
        auto zFloat = z * depthFactor;

        if (activeTexture != texture) {
            flush();
            activeTexture = texture;
        }

        vertices.push_back(x * iwidth - 1.0f);
        vertices.push_back(1.0f - y * iheight);
        vertices.push_back(zFloat);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        vertices.push_back(x * iwidth - 1.0f);
        vertices.push_back(1.0f - (y + h) * iheight);
        vertices.push_back(zFloat);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);

        vertices.push_back((x + w) * iwidth - 1.0f);
        vertices.push_back(1.0f - y * iheight);
        vertices.push_back(zFloat);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);


        vertices.push_back(x * iwidth - 1.0f);
        vertices.push_back(1.0f - (y + h) * iheight);
        vertices.push_back(zFloat);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);

        vertices.push_back((x + w) * iwidth - 1.0f);
        vertices.push_back(1.0f - (y + h) * iheight);
        vertices.push_back(zFloat);
        vertices.push_back(1.0f);
        vertices.push_back(1.0f);

        vertices.push_back((x + w) * iwidth - 1.0f);
        vertices.push_back(1.0f - y * iheight);
        vertices.push_back(zFloat);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
    }

    template<typename SurfaceType>
    void blit(SurfaceType& surface, const BlitSettings& settings) {
        auto texture = std::static_pointer_cast<GLTexture>(surface.texture);
        if (!texture) {
            texture = std::make_shared<GLTexture>();
            surface.texture = std::static_pointer_cast<Texture>(texture);
            surface.setDirty();
        }

        if (surface.isDirty())
            upload(surface);

        push(texture, settings);
    }

    void blit(const BlitSettings& settings) override {
        match::variant(*settings.surface,
                       [&](Surface256& surface){blit(surface, settings);},
                       [&](SurfaceRGBA& surface){blit(surface, settings);});
    }
};
