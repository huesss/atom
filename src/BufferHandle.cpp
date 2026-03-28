#include "BufferHandle.hpp"

BufferHandle::BufferHandle(Type type) : m_type(type) {
    if (m_type == Type::VBO) {
        glGenBuffers(1, &m_id);
    } else {
        glGenVertexArrays(1, &m_id);
    }
}

BufferHandle::~BufferHandle() {
    if (m_id) {
        if (m_type == Type::VBO) {
            glDeleteBuffers(1, &m_id);
        } else {
            glDeleteVertexArrays(1, &m_id);
        }
    }
}

BufferHandle::BufferHandle(BufferHandle&& other) noexcept
    : m_id(other.m_id), m_type(other.m_type) {
    other.m_id = 0;
}

BufferHandle& BufferHandle::operator=(BufferHandle&& other) noexcept {
    if (this != &other) {
        if (m_id) {
            if (m_type == Type::VBO) {
                glDeleteBuffers(1, &m_id);
            } else {
                glDeleteVertexArrays(1, &m_id);
            }
        }
        m_id = other.m_id;
        m_type = other.m_type;
        other.m_id = 0;
    }
    return *this;
}
