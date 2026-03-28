#include "Atom.hpp"

#include <cmath>

void AtomModel::setupHydrogen() {
    orbitAngle = 0.0f;
    particles.clear();
    Particle proton;
    proton.position = glm::vec3(0.0f);
    proton.charge = 1.0f;
    proton.color = glm::vec3(1.0f, 0.92f, 0.2f);
    particles.push_back(proton);
    Particle electron;
    electron.charge = -1.0f;
    electron.color = glm::vec3(0.2f, 0.75f, 1.0f);
    particles.push_back(electron);
    syncElectronFromOrbit();
}

void AtomModel::syncElectronFromOrbit() {
    if (particles.size() < 2) {
        return;
    }
    const float r = static_cast<float>(bohrN * bohrN) * a0;
    particles[0].position = glm::vec3(0.0f);
    particles[1].position =
        glm::vec3(r * std::cos(orbitAngle), r * std::sin(orbitAngle), 0.0f);
}

void AtomModel::updateClassical(float deltaTime) {
    if (particles.size() < 2) {
        setupHydrogen();
    }
    const float omega = orbitSpeed / std::max(1.0f, static_cast<float>(bohrN));
    orbitAngle += omega * deltaTime;
    syncElectronFromOrbit();
}

void AtomModel::fillBohrOrbitPolyline(int bohrN_in, float a0_in, int segments,
                                      std::vector<glm::vec3>& out) {
    out.clear();
    if (segments < 3) {
        segments = 3;
    }
    const float r = static_cast<float>(bohrN_in * bohrN_in) * a0_in;
    out.reserve(static_cast<std::size_t>(segments) + 1);
    for (int i = 0; i <= segments; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(segments) * 6.28318530718f;
        out.push_back(glm::vec3(r * std::cos(t), r * std::sin(t), 0.0f));
    }
}
