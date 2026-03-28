#pragma once

#include "Atom.hpp"
#include "BufferHandle.hpp"
#include "ComputeShader.hpp"
#include "QuantumHydrogen.hpp"
#include "Shader.hpp"
#include "SimpleUI.hpp"
#include "SphereMesh.hpp"

#include <glm/glm.hpp>

#include <deque>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

struct GLFWwindow;
class Camera;

class AtomSimulation {
public:
    AtomSimulation(const std::filesystem::path& shaderRoot);
    ~AtomSimulation();

    void update(float deltaTime, bool keyW, bool keyS, bool keyA, bool keyD,
                bool keyE, bool keyQ, bool keySpace, bool keyLeftCtrl, Camera& camera);
    void render(const Camera& camera);
    void onResize(int width, int height);

    void toggleQuantumMode();
    void applyQuantumNumbers();
    void setPreset(int preset);
    void decrementN();
    void incrementN();
    void decrementL();
    void incrementL();
    void decrementM();
    void incrementM();
    void decreaseParticleCount();
    void increaseParticleCount();
    void toggleVSync(GLFWwindow* w);
    void toggleConsole();
    void adjustTimeScale(float delta);

private:
    AtomModel m_atom;
    HydrogenOrbitalSampler m_sampler;
    bool m_quantumMode = true;

    int m_fbWidth = 1280;
    int m_fbHeight = 720;

    std::filesystem::path m_shaderRoot;
    Shader m_shaderLine;
    Shader m_shaderParticles;
    Shader m_shaderSphere;

    ComputeShader m_computeGenerate;
    ComputeShader m_computeUpdate;
    ComputeShader m_computeInterpolate;

    BufferHandle m_vaoOrbit;
    BufferHandle m_vboOrbit;
    BufferHandle m_vaoCloud;
    BufferHandle m_vboCloudCorner;
    BufferHandle m_ssboCloud;
    BufferHandle m_ssboCloudTarget;

    BufferHandle m_ssboRMid;
    BufferHandle m_ssboRCdf;
    BufferHandle m_ssboThetaMid;
    BufferHandle m_ssboThetaCdf;

    std::vector<glm::vec3> m_orbitPolyline;
    std::uint32_t m_cloudCount = 360000;

    float m_simTime = 0.0f;
    int m_qn = 2;
    int m_ql = 1;
    int m_qm = 0;

    bool m_needsRegeneration = false;
    int m_lastQn = 2;
    int m_lastQl = 1;
    int m_lastQm = 0;

    float m_bufferUpdateAccum = 0.0f;
    float m_bufferUpdateInterval = 0.016f;

    bool m_isInterpolating = false;
    float m_interpolationProgress = 0.0f;
    float m_interpolationDuration = 1.2f;

    bool m_vsyncEnabled = true;
    SimpleUI m_ui;
    bool m_showConsole = false;
    std::deque<std::string> m_consoleLines;
    std::deque<float> m_fpsHistory;
    float m_fpsSmooth = 60.0f;

    float m_timeScale = 1.0f;

    SphereMesh m_sphereMesh;

    void rebuildOrbit();
    void initGpuBuffers();
    void uploadCdfTables();
    void generateCloudGPU();
    void addConsoleMessage(const std::string& msg);
};
