#include "InputManager.hpp"
#include "AtomSimulation.hpp"
#include "Camera.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iomanip>
#include <sstream>

InputManager::InputManager() {
    for (int i = 0; i < 1024; ++i) {
        m_keys[i] = false;
    }
}

void InputManager::setCamera(Camera* camera) {
    m_camera = camera;
}

void InputManager::setSimulation(AtomSimulation* simulation) {
    m_simulation = simulation;
}

bool InputManager::isKeyPressed(int key) const {
    if (key < 0 || key >= 1024) return false;
    return m_keys[key];
}

void InputManager::onKey(GLFWwindow* w, int key, int, int action, int) {
    if (key < 0 || key >= 1024) return;

    if (action == GLFW_PRESS) {
        m_keys[key] = true;

        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(w, GLFW_TRUE);
        }

        if (m_simulation) {
            if (key == GLFW_KEY_TAB) {
                m_simulation->toggleQuantumMode();
            }
            if (key == GLFW_KEY_R) {
                m_simulation->applyQuantumNumbers();
            }
            if (key == GLFW_KEY_1) {
                m_simulation->setPreset(1);
            }
            if (key == GLFW_KEY_2) {
                m_simulation->setPreset(2);
            }
            if (key == GLFW_KEY_3) {
                m_simulation->setPreset(3);
            }
            if (key == GLFW_KEY_LEFT_BRACKET) {
                m_simulation->decrementN();
            }
            if (key == GLFW_KEY_RIGHT_BRACKET) {
                m_simulation->incrementN();
            }
            if (key == GLFW_KEY_COMMA) {
                m_simulation->decrementL();
            }
            if (key == GLFW_KEY_PERIOD) {
                m_simulation->incrementL();
            }
            if (key == GLFW_KEY_SEMICOLON) {
                m_simulation->decrementM();
            }
            if (key == GLFW_KEY_APOSTROPHE) {
                m_simulation->incrementM();
            }
            if (key == GLFW_KEY_MINUS) {
                m_simulation->decreaseParticleCount();
            }
            if (key == GLFW_KEY_EQUAL) {
                m_simulation->increaseParticleCount();
            }
            if (key == GLFW_KEY_V) {
                m_simulation->toggleVSync(w);
            }
            if (key == GLFW_KEY_P) {
                m_simulation->toggleConsole();
            }
        }
    } else if (action == GLFW_RELEASE) {
        m_keys[key] = false;
    }
}

void InputManager::onMouseMove(double x, double y) {
    if (m_camera) {
        m_camera->onMouseMoved(x, y);
    }
}

void InputManager::onScroll(double, double yoffset) {
    if (m_simulation) {
        m_simulation->adjustTimeScale(static_cast<float>(yoffset));
    }
}

void InputManager::onResize(int width, int height) {
    if (m_camera) {
        m_camera->onResize(width, height);
    }
    if (m_simulation) {
        m_simulation->onResize(width, height);
    }
}
