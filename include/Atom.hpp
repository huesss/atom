#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Particle {
    glm::vec3 position{};
    float charge = 0.0f;
    glm::vec3 color{1.0f, 1.0f, 1.0f};
};

struct AtomModel {
    std::vector<Particle> particles;
    int bohrN = 1;
    float a0 = 1.0f;
    float orbitAngle = 0.0f;
    float orbitSpeed = 1.4f;

    void setupHydrogen();
    void updateClassical(float deltaTime);
    void syncElectronFromOrbit();
    static void fillBohrOrbitPolyline(int bohrN, float a0, int segments,
                                      std::vector<glm::vec3>& out);
};
