#include "AtomSimulation.hpp"
#include "Camera.hpp"
#include "InputManager.hpp"
#include "Window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <filesystem>

namespace fs = std::filesystem;

static fs::path resolveShaderRoot(int argc, char** argv) {
    if (argc > 0) {
        std::error_code ec;
        const fs::path tryAbs = fs::absolute(argv[0], ec).parent_path() / "shaders";
        if (fs::is_directory(tryAbs)) {
            return tryAbs;
        }
        const fs::path tryRel = fs::path(argv[0]).parent_path() / "shaders";
        if (fs::is_directory(tryRel)) {
            return fs::absolute(tryRel);
        }
    }
    if (fs::is_directory("shaders")) {
        return fs::absolute("shaders");
    }
    return fs::path("shaders");
}

int main(int argc, char** argv) {
    const fs::path shaderRoot = resolveShaderRoot(argc, argv);

    Window window(1280, 720, "QuantumAtom");
    Camera camera(window.handle(), glm::vec3(20.0f, 14.0f, 20.0f), -48.0f, -32.0f);
    InputManager inputManager;
    AtomSimulation simulation(shaderRoot);

    window.setInputManager(&inputManager);
    inputManager.setCamera(&camera);
    inputManager.setSimulation(&simulation);

    camera.onResize(window.width(), window.height());
    simulation.onResize(window.width(), window.height());

    double lastTime = glfwGetTime();

    while (!window.shouldClose()) {
        const double now = glfwGetTime();
        const float deltaTime = static_cast<float>(now - lastTime);
        lastTime = now;

        simulation.update(deltaTime, inputManager.isKeyPressed(GLFW_KEY_W),
                          inputManager.isKeyPressed(GLFW_KEY_S),
                          inputManager.isKeyPressed(GLFW_KEY_A),
                          inputManager.isKeyPressed(GLFW_KEY_D),
                          inputManager.isKeyPressed(GLFW_KEY_E),
                          inputManager.isKeyPressed(GLFW_KEY_Q),
                          inputManager.isKeyPressed(GLFW_KEY_SPACE),
                          inputManager.isKeyPressed(GLFW_KEY_LEFT_CONTROL), camera);
        simulation.render(camera);
        window.swapBuffers();
        window.pollEvents();
    }

    return 0;
}
