#include "SphereMesh.hpp"

#include <cmath>
#include <glm/glm.hpp>
#include <vector>

SphereMesh::SphereMesh(float radius, int stacks, int sectors) {
    std::vector<float> vertices;
    constexpr float kPi = 3.14159265358979323846f;
    for (int i = 0; i < stacks; ++i) {
        const float t1 = kPi * static_cast<float>(i) / static_cast<float>(stacks);
        const float t2 = kPi * static_cast<float>(i + 1) / static_cast<float>(stacks);
        for (int j = 0; j < sectors; ++j) {
            const float p1 = 2.0f * kPi * static_cast<float>(j) / static_cast<float>(sectors);
            const float p2 = 2.0f * kPi * static_cast<float>(j + 1) / static_cast<float>(sectors);
            const auto p = [&](float t, float p_) {
                return glm::vec3(radius * std::sin(t) * std::cos(p_), radius * std::cos(t),
                                 radius * std::sin(t) * std::sin(p_));
            };
            const glm::vec3 v1 = p(t1, p1);
            const glm::vec3 v2 = p(t1, p2);
            const glm::vec3 v3 = p(t2, p1);
            const glm::vec3 v4 = p(t2, p2);
            const float tri1[] = {v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z};
            const float tri2[] = {v2.x, v2.y, v2.z, v4.x, v4.y, v4.z, v3.x, v3.y, v3.z};
            vertices.insert(vertices.end(), tri1, tri1 + 9);
            vertices.insert(vertices.end(), tri2, tri2 + 9);
        }
    }
    m_count = static_cast<GLsizei>(vertices.size() / 3);
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

SphereMesh::~SphereMesh() {
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
    }
}

SphereMesh::SphereMesh(SphereMesh&& other) noexcept {
    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_count = other.m_count;
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_count = 0;
}

SphereMesh& SphereMesh::operator=(SphereMesh&& other) noexcept {
    if (this != &other) {
        if (m_vbo) {
            glDeleteBuffers(1, &m_vbo);
        }
        if (m_vao) {
            glDeleteVertexArrays(1, &m_vao);
        }
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_count = other.m_count;
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_count = 0;
    }
    return *this;
}

void SphereMesh::draw() const {
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_count);
    glBindVertexArray(0);
}
