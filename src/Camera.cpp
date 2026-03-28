#include "Camera.hpp"

#include <GLFW/glfw3.h>
#include <cmath>

Camera::Camera(GLFWwindow* w, glm::vec3 position, float yawDeg, float pitchDeg)
    : m_pos(position), m_yaw(yawDeg), m_pitch(pitchDeg) {
    int ww = 0, wh = 0;
    glfwGetFramebufferSize(w, &ww, &wh);
    if (wh > 0) {
        m_aspect = static_cast<float>(ww) / static_cast<float>(wh);
    }
}

void Camera::processFrame(float deltaTime, bool moveForward, bool moveBack, bool moveLeft, bool moveRight,
                          bool moveUp, bool moveDown, bool worldFlyUp, bool worldFlyDown) {
    const glm::vec3 front = glm::normalize(glm::vec3(
        cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
        sin(glm::radians(m_pitch)),
        sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))));
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
    if (glm::length(right) < 1e-4f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    const glm::vec3 up = glm::normalize(glm::cross(right, front));
    const float v = m_moveSpeed * deltaTime;
    if (moveForward) {
        m_pos += front * v;
    }
    if (moveBack) {
        m_pos -= front * v;
    }
    if (moveRight) {
        m_pos += right * v;
    }
    if (moveLeft) {
        m_pos -= right * v;
    }
    if (moveUp) {
        m_pos += up * v;
    }
    if (moveDown) {
        m_pos -= up * v;
    }
    if (worldFlyUp) {
        m_pos += worldUp * v;
    }
    if (worldFlyDown) {
        m_pos -= worldUp * v;
    }
}

void Camera::onMouseMoved(double x, double y) {
    if (m_firstMouse) {
        m_lastX = x;
        m_lastY = y;
        m_firstMouse = false;
        return;
    }
    const double dx = x - m_lastX;
    const double dy = m_lastY - y;
    m_lastX = x;
    m_lastY = y;
    m_yaw += static_cast<float>(dx) * m_mouseSens;
    m_pitch += static_cast<float>(dy) * m_mouseSens;
    if (m_pitch > 89.0f) {
        m_pitch = 89.0f;
    }
    if (m_pitch < -89.0f) {
        m_pitch = -89.0f;
    }
}

void Camera::onResize(int width, int height) {
    if (height <= 0) {
        return;
    }
    m_aspect = static_cast<float>(width) / static_cast<float>(height);
}

glm::mat4 Camera::viewMatrix() const {
    const glm::vec3 front = glm::normalize(glm::vec3(
        cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
        sin(glm::radians(m_pitch)),
        sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))));
    return glm::lookAt(m_pos, m_pos + front, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::projectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspect, 0.05f, 500.0f);
}
