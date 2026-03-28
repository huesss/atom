#pragma once

#include <glad/gl.h>
#include <utility>

class BufferHandle {
public:
    enum class Type { VBO, VAO };

    BufferHandle(Type type);
    ~BufferHandle();

    BufferHandle(const BufferHandle&) = delete;
    BufferHandle& operator=(const BufferHandle&) = delete;

    BufferHandle(BufferHandle&& other) noexcept;
    BufferHandle& operator=(BufferHandle&& other) noexcept;

    GLuint id() const { return m_id; }
    operator GLuint() const { return m_id; }

private:
    GLuint m_id = 0;
    Type m_type;
};
