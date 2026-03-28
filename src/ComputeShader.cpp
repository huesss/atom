#include "ComputeShader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

ComputeShader::ComputeShader(const std::string& computePath) {
    const std::string csrc = readFile(computePath);
    const GLuint cs = compile(csrc.c_str());
    m_program = link(cs);
    glDeleteShader(cs);
}

ComputeShader::~ComputeShader() {
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

void ComputeShader::use() const {
    glUseProgram(m_program);
}

void ComputeShader::dispatch(GLuint groupsX, GLuint groupsY, GLuint groupsZ) const {
    glDispatchCompute(groupsX, groupsY, groupsZ);
}

void ComputeShader::wait() const {
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void ComputeShader::setInt(const std::string& name, int v) const {
    glUniform1i(uniformLoc(name), v);
}

void ComputeShader::setUint(const std::string& name, unsigned int v) const {
    glUniform1ui(uniformLoc(name), v);
}

void ComputeShader::setFloat(const std::string& name, float v) const {
    glUniform1f(uniformLoc(name), v);
}

std::string ComputeShader::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "compute shader file: " << path << '\n';
        return {};
    }
    std::stringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

GLuint ComputeShader::compile(const char* src) {
    const GLuint sh = glCreateShader(GL_COMPUTE_SHADER);
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

GLuint ComputeShader::link(GLuint cs) {
    const GLuint p = glCreateProgram();
    glAttachShader(p, cs);
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

GLint ComputeShader::uniformLoc(const std::string& name) const {
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    const GLint loc = glGetUniformLocation(m_program, name.c_str());
    m_uniformCache[name] = loc;
    return loc;
}
