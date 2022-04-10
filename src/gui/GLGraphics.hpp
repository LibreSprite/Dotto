// Copyright (c) 2021 LibreSprite Authors (cf. AUTHORS.md)
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#elif defined(__WINDOWS__)
  #include <windef.h>
  #include <GL/glew.h>
  #include <GL/gl.h>
#else
  #include <GL/gl.h>
  #include <GLES3/gl3.h>
#endif

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
    Rect dirtyRegion{0, 0, ~U32{}, ~U32{}};

    GLTexture() {
        glGenTextures(1, &id);
    }

    ~GLTexture() {
        if (id)
            glDeleteTextures(1, &id);
    }

    void setDirty(const Rect& region) override {
        dirtyRegion.expand(region);
    }

    void bind(U32 target) {
        glBindTexture(target, id);
    }
};

class GLGraphics : public Graphics, public std::enable_shared_from_this<GLGraphics> {
public:
    U32 VBO = 0;
    U32 VAO = 0;
    U32 shader = 0;
    Vector<F32> vertices;
    std::shared_ptr<GLTexture> activeTexture;

    F32 iwidth, iheight;
    S32 width, height;

    U32 empty = 0;

    void init(const String& version) {
#if defined(__WINDOWS__)
        glewInit();
#endif

        glGenBuffers(1, &VBO);
        glGenVertexArrays(1, &VAO);

        glGenTextures(1, &empty);
        glBindTexture(GL_TEXTURE_2D, empty);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        auto color = Color{0xFF, 0xFF, 0xFF, 0xFF}.toU32();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);

        auto vertexShader = compile(GL_VERTEX_SHADER,
            "#version " + version + "\n"
            "layout(location=0) in vec3 position;\n"
            "layout(location=1) in vec2 srcUV;\n"
            "layout(location=2) in vec4 diffuse;\n"
            "out vec2 uv;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "   gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
            "   uv = srcUV;\n"
            "   color = diffuse;\n"
            "}");

        auto fragmentShader = compile(GL_FRAGMENT_SHADER,
            "#version " + version + "\n"
            "precision mediump float;"
            "in vec2 uv;\n"
            "in vec4 color;\n"
            "out vec4 FragColor;\n"
            "uniform sampler2D surface;\n"
            "void main() {\n"
            "    FragColor = texture(surface, uv) * color;\n"
            "}");

        shader = link(vertexShader, fragmentShader);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    GLuint rawFB = 0;
    GLuint RBO = 0;
    std::shared_ptr<Surface> rawSurface;
    std::shared_ptr<Surface> renderTarget;

    void setupRawRenderTarget() {
        static U32 oldSize = 0;
        U32 size = U32(width * scale) * U32(height * scale);
        bool needsResize = size != oldSize;
        oldSize = size;

        if (!rawSurface) {
            glGenFramebuffers(1, &rawFB);
            glGenRenderbuffers(1, &RBO);
            rawSurface = std::make_shared<Surface>();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, rawFB);

        std::shared_ptr<GLTexture> texture = getTexture(*rawSurface);

        if (needsResize) {
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width * scale, height * scale);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            rawSurface->resize(width * scale, height * scale);
            upload(*rawSurface, texture.get());
        }

        texture->bind(GL_TEXTURE_2D);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
        renderTarget = rawSurface;
    }

    U32 compile(U32 type, const String& source) {
        U32 shader = glCreateShader(type);
        auto str = source.c_str();
        glShaderSource(shader, 1, &str, NULL);
        glCompileShader(shader);

        int  success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            infoLog[0] = 0;
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            logE("ERROR::SHADER::COMPILATION_FAILED ", type, " ", infoLog);
            glDeleteShader(shader);
            shader = 0;
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
        textures.clear();
        if (empty)
            glDeleteTextures(1, &empty);
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
        setupRawRenderTarget();
        glViewport(0, 0, width * scale, height * scale);
        glClearColor(clearColor.r/255.0f,
                     clearColor.g/255.0f,
                     clearColor.b/255.0f,
                     clearColor.a/255.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void end() {
        flush();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        write();
    }

    U32 currentBufferSize = 0;
    U32 currentShader = 0;

    void flush() {
        if (vertices.empty()) {
            return;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        U32 requiredSize = vertices.size() * sizeof(vertices[0]);
        if (requiredSize > currentBufferSize) {
            glBufferData(GL_ARRAY_BUFFER, requiredSize, vertices.data(), GL_STREAM_DRAW);
            currentBufferSize = requiredSize;

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(5 * sizeof(float)));
            glEnableVertexAttribArray(2);
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, requiredSize, vertices.data());
        }

        if (currentShader != shader) {
            currentShader = shader;
            glUseProgram(shader);
        }

        if (activeTexture) {
            activeTexture->bind(GL_TEXTURE_2D);
        } else {
            glBindTexture(GL_TEXTURE_2D, empty);
        }
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 9);

        vertices.clear();
        activeTexture.reset();
    }

    void upload(Surface& surface, GLTexture* texture) {
        texture->bind(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // TODO: Use dirtyRegion to upload only what changed
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     surface.width(),
                     surface.height(),
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     surface.data());
        texture->width = surface.width();
        texture->iwidth = 1.0f / texture->width;
        texture->height = surface.height();
        texture->iheight = 1.0f / texture->height;
        // glGenerateMipmap(GL_TEXTURE_2D);
        texture->dirtyRegion = Rect{};
    }

    struct Vertex {
        F32 x;
        F32 y;
        F32 z;
        F32 u;
        F32 v;
        F32 r;
        F32 g;
        F32 b;
        F32 a;
        bool flip;
    };

    void push(const Vertex& vtx) {
        constexpr const F32 depthFactor = 0.0000001f;
        vertices.push_back(vtx.x * iwidth - 1.0f);
        if (vtx.flip) {
            vertices.push_back(vtx.y * iheight - 1.0f);
        } else {
            vertices.push_back(1.0f - vtx.y * iheight);
        }
        vertices.push_back(vtx.z * depthFactor);
        vertices.push_back(vtx.u);
        vertices.push_back(vtx.v);
        vertices.push_back(vtx.r);
        vertices.push_back(vtx.g);
        vertices.push_back(vtx.b);
        vertices.push_back(vtx.a);
    }

    struct Rectf {
        F32 x, y, w, h;
        F32 u0, v0, u1, v1;
        F32 r, g, b, a;
        bool flip;
    };

    bool debug = false;

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
            y2 <= clip.y) {
            if (debug)
                logI("Texture clipped away");
            return;
        }

        if (x1 < clip.x) {
            if (rect.w) {
                u0 += (clip.x - x1) / rect.w;
            }
            x1 = clip.x;
        }
        if (x2 > clip.right()) {
            if (rect.w) {
                u1 -= (x2 - clip.right()) / rect.w;
            }
            x2 = clip.right();
        }

        if (y1 < clip.y) {
            if (rect.h) {
                v0 += (clip.y - y1) / rect.h;
            }
            y1 = clip.y;
        }
        if (y2 > clip.bottom()) {
            if (rect.h) {
                v1 -= (y2 - clip.bottom()) / rect.h;
            }
            y2 = clip.bottom();
        }

        if (debug)
            logI("Pushing ", x1, " ", y1, " => ", x2, " ", y2, " to ", shared_from_this(), " ", activeTexture);

        push({x1, y1, z, u0, v0, rect.r, rect.g, rect.b, rect.a, rect.flip});
        push({x1, y2, z, u0, v1, rect.r, rect.g, rect.b, rect.a, rect.flip});
        push({x2, y1, z, u1, v0, rect.r, rect.g, rect.b, rect.a, rect.flip});
        push({x1, y2, z, u0, v1, rect.r, rect.g, rect.b, rect.a, rect.flip});
        push({x2, y2, z, u1, v1, rect.r, rect.g, rect.b, rect.a, rect.flip});
        push({x2, y1, z, u1, v0, rect.r, rect.g, rect.b, rect.a, rect.flip});
    }

    void push(std::shared_ptr<GLTexture>& texture, const BlitSettings& settings) {
        debug = settings.debug;
        F32 x = settings.destination.x;
        F32 y = settings.destination.y;
        F32 z = settings.zIndex;
        F32 w = settings.destination.width;
        F32 h = settings.destination.height;
        F32 r = settings.multiply.r / 255.0f;
        F32 g = settings.multiply.g / 255.0f;
        F32 b = settings.multiply.b / 255.0f;
        F32 a = settings.multiply.a / 255.0f * alpha;

        if (a <= 0)
            return;

        if (!clip.overlaps(settings.destination)) {
            if (debug)
                logI("Killed by the clip");
            return;
        }

        if (activeTexture != texture) {
            flush();
            activeTexture = texture;
        }
        
        F32 sW = settings.nineSlice.width;
        F32 sH = settings.nineSlice.height;
        S32 textureWidth = texture ? texture->width : 1;
        S32 textureHeight = texture ? texture->height : 1;
        F32 textureIwidth = texture ? texture->iwidth : 1;
        F32 textureIheight = texture ? texture->iheight : 1;

        if (sW == 0 && settings.nineSlice.x != 0)
            sW = textureWidth - settings.nineSlice.x * 2;
        if (sH == 0 && settings.nineSlice.y != 0)
            sH = textureHeight - settings.nineSlice.y * 2;

        if (sW <= 0 || sH <= 0) {
            push(z, {
                    x, y,
                    w, h,
                    settings.source.x / F32(textureWidth), settings.source.y / F32(textureHeight),
                    settings.source.right() / F32(textureWidth), settings.source.bottom() / F32(textureHeight),
                    r, g, b, a,
                    settings.flip
                });
        } else {
            F32 sX = settings.nineSlice.x;
            F32 sY = settings.nineSlice.y;
            F32 nsX = sX * textureIwidth;
            F32 nsY = sY * textureIheight;
            F32 nsW = sW * textureIwidth;
            F32 nsH = sH * textureIheight;
            F32 rW = textureWidth - sW - sX;
            F32 rH = textureHeight - sH - sY;

            push(z, {x,          y, sX,          sY, 0.0f,      0.0f, nsX,       nsY, r, g, b, a});
            push(z, {x + sX,     y, w - sX - rW, sY, nsX,       0.0f, nsX + nsW, nsY, r, g, b, a});
            push(z, {x + w - rW, y, rW,          sY, nsX + nsW, 0.0f, 1.0f,      nsY, r, g, b, a});

            y += sY;
            push(z, {x,        y, sX,      h-sY-rH, 0.0f,      nsY, nsX,       nsY + nsH, r, g, b, a});
            push(z, {x + sX,   y, w-sX-rW, h-sY-rH, nsX,       nsY, nsX + nsW, nsY + nsH, r, g, b, a});
            push(z, {x + w-rW, y, rW,      h-sY-rH, nsX + nsW, nsY, 1.0f,      nsY + nsH, r, g, b, a});

            y += h - sY - rH;
            push(z, {x,          y, sX,          rH, 0.0f,      nsY + nsH, nsX, 1.0f, r, g, b, a});
            push(z, {x + sX,     y, w - sX - rW, rH, nsX,       nsY + nsH, nsX + nsW, 1.0f, r, g, b, a});
            push(z, {x + w - rW, y, rW,          rH, nsX + nsW, nsY + nsH, 1.0f, 1.0f, r, g, b, a});
        }
    }

    Vector<fork_ptr<Texture>> textures;

    std::shared_ptr<GLTexture> getTexture(Surface& surface) {
        auto texture = surface.info().get<GLTexture>(this);

        if (!texture) {
            texture = std::make_shared<GLTexture>();
            fork_ptr<Texture> ptr{std::static_pointer_cast<Texture>(texture)};
            textures.push_back(ptr);
            surface.info().set(this, std::move(ptr));
        }

        return texture;
    }

    void blit(const BlitSettings& settings) override {
        std::shared_ptr<GLTexture> texture;
        if (settings.surface) {
            auto& surface = *settings.surface;
            texture = getTexture(surface);
            if (!texture->dirtyRegion.empty()) {
                if (settings.debug)
                    logI("Uploading surface");
                upload(surface, texture.get());
            }
        } else if (settings.multiply.a == 0) {
            return; // no texture + no color = no op
        }

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

    Surface* read() override {
        glReadPixels(0, 0, width * scale, height * scale, GL_RGBA, GL_UNSIGNED_BYTE, renderTarget->data());
        renderTarget->setDirty(renderTarget->rect());
        return renderTarget.get();
    }

    void write() override {
        if (!rawSurface)
            return;
        Rect rect(0, 0, width * scale, height * scale);
        Rect dest(0, 0, width, height);
        Rect slice;
        setClipRect(rect);
        alpha = 1.0f;
        blit({
                .surface     = rawSurface,
                .source      = rect,
                .destination = dest,
                .nineSlice   = slice,
                .zIndex      = 1000000,
                .multiply    = Color{255, 255, 255, 255},
                .debug       = false,
                .flip        = true
            });
        flush();
    }
};

