#include "Window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "InputManager.hpp"

#include <iostream>

Window::Window(int width, int height, const char* title)
    : m_width(width), m_height(height) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {
        std::cerr << "gladLoadGL failed\n";
        glfwTerminate();
        return;
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwGetFramebufferSize(m_window, &m_width, &m_height);
    glViewport(0, 0, m_width, m_height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::setInputManager(InputManager* manager) {
    m_inputManager = manager;
}

void Window::framebufferSizeCallback(GLFWwindow* w, int width, int height) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!window || !window->m_inputManager) return;
    
    window->m_width = width;
    window->m_height = height;
    glViewport(0, 0, width, height);
    window->m_inputManager->onResize(width, height);
}

void Window::keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!window || !window->m_inputManager) return;
    window->m_inputManager->onKey(w, key, scancode, action, mods);
}

void Window::cursorPosCallback(GLFWwindow* w, double x, double y) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!window || !window->m_inputManager) return;
    window->m_inputManager->onMouseMove(x, y);
}

void Window::scrollCallback(GLFWwindow* w, double xoffset, double yoffset) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!window || !window->m_inputManager) return;
    window->m_inputManager->onScroll(xoffset, yoffset);
}
