#pragma once

#include <glad/gl.h>

struct GLFWwindow;

class InputManager;

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    
    GLFWwindow* handle() const { return m_window; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    
    void setInputManager(InputManager* manager);
    InputManager* inputManager() const { return m_inputManager; }

private:
    GLFWwindow* m_window = nullptr;
    int m_width;
    int m_height;
    InputManager* m_inputManager = nullptr;

    static void framebufferSizeCallback(GLFWwindow* w, int width, int height);
    static void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* w, double x, double y);
    static void scrollCallback(GLFWwindow* w, double xoffset, double yoffset);
};
