#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include "Log.hpp"

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#elif defined(__WINDOWS__)
  #include <windef.h>
  #include <GL/glew.h>
  #include <GL/gl.h>
#elif defined(ANDROID)
  #include <GLES/gl.h>
  #include <GLES3/gl3.h>
#else
  #include <GL/gl.h>
  #include <GLES3/gl3.h>
#endif

class GLShader {
public:
    static inline uint32_t major{};
    static inline uint32_t minor{};
    static inline std::string profile;

    uint32_t program{};

    struct Attribute {
        GLuint index;
        GLenum type;
        GLint size;
    };
    std::unordered_map<std::string, Attribute> attributes;
    std::unordered_map<std::string, Attribute> uniforms;

    GLShader(uint32_t program) : program{program} {
        constexpr const GLsizei bufSize = 32; // maximum name length
        GLchar name[bufSize]; // variable name in GLSL
        GLsizei length; // name length
        GLint count{};
        Attribute tmp;

        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
        for (GLint i = 0; i < count; i++) {
            tmp.index = i;
            glGetActiveAttrib(program, tmp.index, bufSize, &length, &tmp.size, &tmp.type, name);
            attributes[name] = tmp;
        }

        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
        for (GLint i = 0; i < count; i++) {
            glGetActiveUniform(program, i, bufSize, &length, &tmp.size, &tmp.type, name);
            tmp.index = glGetUniformLocation(program, name);
            uniforms[name] = tmp;
        }
    }

    ~GLShader() {
        glDeleteProgram(program);
    }

    void use() {
      glUseProgram(program);
    }

    static std::shared_ptr<GLShader> create(const std::string& vtxSrc, const std::string& frgSrc) {
	std::string version = std::to_string(major) + std::to_string(minor) + "0 " + profile;
        auto vtx = compile(GL_VERTEX_SHADER, "#version " + version + "\n" + vtxSrc);
        if (!vtx) {
            return nullptr;
	}

        auto frg = compile(GL_FRAGMENT_SHADER, "#version " + version + "\n" + frgSrc);
        if (!frg) {
            glDeleteShader(vtx);
            return nullptr;
        }

        auto prg = link(vtx, frg);
        glDeleteShader(vtx);
        glDeleteShader(frg);

        return prg ? std::make_shared<GLShader>(prg) : nullptr;
    }

    static uint32_t compile(uint32_t type, const std::string& source) {
        uint32_t shader = glCreateShader(type);
        auto str = source.c_str();
        glShaderSource(shader, 1, &str, NULL);
        glCompileShader(shader);

        int  success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            infoLog[0] = 0;
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            LOG("ERROR::SHADER::COMPILATION_FAILED ", type, " ", infoLog);
	    LOG("In shader:");
	    LOG(source);
	    LOG();
            glDeleteShader(shader);
            shader = 0;
        }

        return shader;
    }

    static uint32_t link(uint32_t vertexShader, uint32_t fragmentShader) {
        uint32_t program = glCreateProgram();
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
            LOG("ERROR::SHADER::LINK_FAILED ", infoLog);
        }
        return program;
    }
};
