#pragma once

#include <glad/gl.h>

class SphereMesh {
public:
    SphereMesh(float radius, int stacks, int sectors);
    ~SphereMesh();

    SphereMesh(const SphereMesh&) = delete;
    SphereMesh& operator=(const SphereMesh&) = delete;
    SphereMesh(SphereMesh&& other) noexcept;
    SphereMesh& operator=(SphereMesh&& other) noexcept;

    void draw() const;
    GLsizei triangleVertexCount() const { return m_count; }
    GLuint geometryVbo() const { return m_vbo; }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLsizei m_count = 0;
};
