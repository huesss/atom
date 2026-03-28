#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLFWwindow;

class Camera {
public:
    explicit Camera(GLFWwindow* w, glm::vec3 position, float yawDeg, float pitchDeg);

    void processFrame(float deltaTime, bool moveForward, bool moveBack, bool moveLeft,
                      bool moveRight, bool moveUp, bool moveDown, bool worldFlyUp,
                      bool worldFlyDown);
    void onMouseMoved(double x, double y);
    void onResize(int width, int height);

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;

    glm::vec3 position() const { return m_pos; }
    float verticalFovDegrees() const { return m_fov; }

private:
    glm::vec3 m_pos;
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_fov = 60.0f;
    float m_aspect = 16.0f / 9.0f;
    double m_lastX = 0.0;
    double m_lastY = 0.0;
    bool m_firstMouse = true;
    float m_moveSpeed = 24.0f;
    float m_mouseSens = 0.085f;
};
