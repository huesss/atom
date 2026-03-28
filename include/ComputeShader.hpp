#pragma once

#include <glad/gl.h>
#include <string>
#include <unordered_map>

class ComputeShader {
public:
    explicit ComputeShader(const std::string& computePath);
    ~ComputeShader();

    void use() const;
    void dispatch(GLuint groupsX, GLuint groupsY, GLuint groupsZ) const;
    void wait() const;

    void setInt(const std::string& name, int v) const;
    void setUint(const std::string& name, unsigned int v) const;
    void setFloat(const std::string& name, float v) const;

    GLuint programId() const { return m_program; }

private:
    GLuint m_program = 0;
    static std::string readFile(const std::string& path);
    static GLuint compile(const char* src);
    static GLuint link(GLuint cs);
    GLint uniformLoc(const std::string& name) const;
    mutable std::unordered_map<std::string, GLint> m_uniformCache;
};
