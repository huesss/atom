#pragma once

#include <glad/gl.h>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void use() const;
    void setMat4(const std::string& name, const float* value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setFloat(const std::string& name, float v) const;

    GLuint programId() const { return m_program; }

private:
    GLuint m_program = 0;
    static std::string readFile(const std::string& path);
    static GLuint compile(GLenum type, const char* src);
    static GLuint link(GLuint vs, GLuint fs);
    GLint uniformLoc(const std::string& name) const;
    mutable std::unordered_map<std::string, GLint> m_uniformCache;
};
