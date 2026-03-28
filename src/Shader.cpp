#include "Shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string vsrc = readFile(vertexPath);
    const std::string fsrc = readFile(fragmentPath);
    const GLuint vs = compile(GL_VERTEX_SHADER, vsrc.c_str());
    const GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc.c_str());
    m_program = link(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() {
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

void Shader::use() const {
    glUseProgram(m_program);
}

void Shader::setMat4(const std::string& name, const float* value) const {
    glUniformMatrix4fv(uniformLoc(name), 1, GL_FALSE, value);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(uniformLoc(name), x, y, z);
}

void Shader::setFloat(const std::string& name, float v) const {
    glUniform1f(uniformLoc(name), v);
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "shader file: " << path << '\n';
        return {};
    }
    std::stringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

GLuint Shader::compile(GLenum type, const char* src) {
    const GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
        std::cerr << log << '\n';
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

GLuint Shader::link(GLuint vs, GLuint fs) {
    const GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        std::cerr << log << '\n';
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

GLint Shader::uniformLoc(const std::string& name) const {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    const GLint loc = glGetUniformLocation(m_program, name.c_str());
    m_uniformCache[name] = loc;
    return loc;
}
