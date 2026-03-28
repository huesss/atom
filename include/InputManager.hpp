#pragma once

struct GLFWwindow;

class Camera;
class AtomSimulation;

class InputManager {
public:
    InputManager();

    void setCamera(Camera* camera);
    void setSimulation(AtomSimulation* simulation);

    void onKey(GLFWwindow* w, int key, int scancode, int action, int mods);
    void onMouseMove(double x, double y);
    void onScroll(double xoffset, double yoffset);
    void onResize(int width, int height);

    bool isKeyPressed(int key) const;

private:
    Camera* m_camera = nullptr;
    AtomSimulation* m_simulation = nullptr;
    bool m_keys[1024]{};
};
