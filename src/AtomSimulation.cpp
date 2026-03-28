#include "AtomSimulation.hpp"
#include "Camera.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

AtomSimulation::AtomSimulation(const std::filesystem::path& shaderRoot)
    : m_sampler(2, 1, 0),
      m_shaderRoot(shaderRoot),
      m_shaderLine((shaderRoot / "line.vert").string(), (shaderRoot / "line.frag").string()),
      m_shaderParticles((shaderRoot / "quantum_point.vert").string(),
                        (shaderRoot / "quantum_point.frag").string()),
      m_shaderSphere((shaderRoot / "sphere.vert").string(), (shaderRoot / "sphere.frag").string()),
      m_computeGenerate((shaderRoot / "quantum_generate.comp").string()),
      m_computeUpdate((shaderRoot / "quantum_update.comp").string()),
      m_computeInterpolate((shaderRoot / "quantum_interpolate.comp").string()),
      m_vaoOrbit(BufferHandle::Type::VAO),
      m_vboOrbit(BufferHandle::Type::VBO),
      m_vaoCloud(BufferHandle::Type::VAO),
      m_vboCloudCorner(BufferHandle::Type::VBO),
      m_ssboCloud(BufferHandle::Type::VBO),
      m_ssboCloudTarget(BufferHandle::Type::VBO),
      m_ssboRMid(BufferHandle::Type::VBO),
      m_ssboRCdf(BufferHandle::Type::VBO),
      m_ssboThetaMid(BufferHandle::Type::VBO),
      m_ssboThetaCdf(BufferHandle::Type::VBO),
      m_sphereMesh(1.0f, 14, 14) {
    m_atom.bohrN = m_qn;
    m_atom.setupHydrogen();
    m_sampler.setQuantumNumbers(m_qn, m_ql, m_qm);

    initGpuBuffers();
    rebuildOrbit();
    uploadCdfTables();
    generateCloudGPU();

    m_lastQn = m_qn;
    m_lastQl = m_ql;
    m_lastQm = m_qm;
    m_needsRegeneration = false;

    const std::string fontPath =
        (m_shaderRoot.parent_path() / "fonts" / "Inter-Regular.ttf").string();
    m_ui.init(m_fbWidth, m_fbHeight, fontPath);

    addConsoleMessage("=== Quantum Atom Visualizer ===");
    addConsoleMessage("Press P to toggle debug log");
    addConsoleMessage("Controls: TAB=mode, 1/2/3=presets, V=vsync");
    addConsoleMessage("Mouse wheel to adjust time scale");
    addConsoleMessage("System initialized successfully");
}

AtomSimulation::~AtomSimulation() {}

void AtomSimulation::initGpuBuffers() {
    glBindVertexArray(m_vaoOrbit);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboOrbit);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4096, nullptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

    const glm::vec3 corner(0.0f);
    glBindVertexArray(m_vaoCloud);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboCloudCorner);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &corner.x, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 0);

    const GLsizeiptr cloudBytes = static_cast<GLsizeiptr>(m_cloudCount * 2 * sizeof(glm::vec4));
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboCloud);
    glBufferData(GL_SHADER_STORAGE_BUFFER, cloudBytes, nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboCloudTarget);
    glBufferData(GL_SHADER_STORAGE_BUFFER, cloudBytes, nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_ssboCloud);
    const GLsizei stride = static_cast<GLsizei>(2 * sizeof(glm::vec4));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, nullptr);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(sizeof(glm::vec4)));
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

void AtomSimulation::uploadCdfTables() {
    const auto& rMid = m_sampler.rMid();
    const auto& rCdf = m_sampler.rCdf();
    const auto& thetaMid = m_sampler.thetaMid();
    const auto& thetaCdf = m_sampler.thetaCdf();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboRMid);
    glBufferData(GL_SHADER_STORAGE_BUFFER, rMid.size() * sizeof(float), rMid.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboRCdf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, rCdf.size() * sizeof(float), rCdf.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboThetaMid);
    glBufferData(GL_SHADER_STORAGE_BUFFER, thetaMid.size() * sizeof(float), thetaMid.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboThetaCdf);
    glBufferData(GL_SHADER_STORAGE_BUFFER, thetaCdf.size() * sizeof(float), thetaCdf.data(), GL_STATIC_DRAW);
}

void AtomSimulation::generateCloudGPU() {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboCloudTarget);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboRMid);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ssboRCdf);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_ssboThetaMid);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_ssboThetaCdf);

    m_computeGenerate.use();
    m_computeGenerate.setUint("uSeed", static_cast<unsigned int>(std::random_device{}()));
    m_computeGenerate.setUint("uCount", m_cloudCount);
    m_computeGenerate.setFloat("uTimePhase", m_simTime);
    m_computeGenerate.setFloat("uRMax", m_sampler.rMax());
    m_computeGenerate.setInt("uN", m_sampler.n());
    m_computeGenerate.setInt("uL", m_sampler.l());
    m_computeGenerate.setInt("uMmag", m_sampler.mmag());
    m_computeGenerate.setFloat("uA0", m_sampler.a0());

    const GLuint groups = (m_cloudCount + 255) / 256;
    m_computeGenerate.dispatch(groups, 1, 1);
    m_computeGenerate.wait();

    m_isInterpolating = true;
    m_interpolationProgress = 0.0f;
}

void AtomSimulation::rebuildOrbit() {
    AtomModel::fillBohrOrbitPolyline(m_atom.bohrN, m_atom.a0, 220, m_orbitPolyline);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboOrbit);
    const GLsizeiptr bytes =
        static_cast<GLsizeiptr>(m_orbitPolyline.size() * sizeof(glm::vec3));
    glBufferData(GL_ARRAY_BUFFER, bytes, m_orbitPolyline.data(), GL_STATIC_DRAW);
}

void AtomSimulation::addConsoleMessage(const std::string& msg) {
    m_consoleLines.push_back(msg);
    if (m_consoleLines.size() > 50) {
        m_consoleLines.pop_front();
    }
}

void AtomSimulation::update(float deltaTime, bool keyW, bool keyS, bool keyA, bool keyD,
                             bool keyE, bool keyQ, bool keySpace, bool keyLeftCtrl,
                             Camera& camera) {
    m_simTime += deltaTime;

    const float currentFps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 60.0f;
    m_fpsHistory.push_back(currentFps);
    if (m_fpsHistory.size() > 30) {
        m_fpsHistory.pop_front();
    }
    float fpsSum = 0.0f;
    for (float f : m_fpsHistory) {
        fpsSum += f;
    }
    m_fpsSmooth = fpsSum / static_cast<float>(m_fpsHistory.size());

    const float scaledDt = deltaTime * m_timeScale;

    camera.processFrame(deltaTime, keyW, keyS, keyA, keyD, keyE, keyQ, keySpace, keyLeftCtrl);

    m_atom.updateClassical(scaledDt);

    if (m_quantumMode) {
        if (m_needsRegeneration) {
            uploadCdfTables();
            generateCloudGPU();
            m_needsRegeneration = false;
            m_bufferUpdateAccum = 0.0f;
        }

        if (m_isInterpolating) {
            m_interpolationProgress += deltaTime / m_interpolationDuration;
            
            if (m_interpolationProgress >= 1.0f) {
                m_interpolationProgress = 1.0f;
                m_isInterpolating = false;
            }

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboCloud);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_ssboCloudTarget);

            m_computeInterpolate.use();
            m_computeInterpolate.setFloat("uLerpFactor", m_interpolationProgress);

            const GLuint groups = (m_cloudCount + 255) / 256;
            m_computeInterpolate.dispatch(groups, 1, 1);
            m_computeInterpolate.wait();
        } else {
            m_bufferUpdateAccum += scaledDt;
            if (m_bufferUpdateAccum >= m_bufferUpdateInterval) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssboCloud);
                
                m_computeUpdate.use();
                m_computeUpdate.setFloat("uDeltaTime", m_bufferUpdateAccum);
                m_computeUpdate.setFloat("uSimTime", m_simTime);
                m_computeUpdate.setInt("uM", m_qm);
                m_computeUpdate.setInt("uN", m_qn);

                const GLuint groups = (m_cloudCount + 255) / 256;
                m_computeUpdate.dispatch(groups, 1, 1);
                m_computeUpdate.wait();

                m_bufferUpdateAccum = 0.0f;
            }
        }
    }
}

void AtomSimulation::render(const Camera& camera) {
    glClearColor(0.05f, 0.06f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 proj = camera.projectionMatrix();
    const glm::mat4 view = camera.viewMatrix();
    const glm::mat4 vp = proj * view;

    const glm::vec3 camPos = camera.position();
    const float protonScale = 0.58f;
    const float electronScale =
        0.17f + 0.035f * std::sqrt(static_cast<float>(m_atom.bohrN));
    const glm::vec3 protonColor(1.0f, 0.92f, 0.2f);
    const glm::vec3 electronColor(0.2f, 0.75f, 1.0f);

    if (m_quantumMode) {
        glDepthMask(GL_FALSE);
        m_shaderParticles.use();
        m_shaderParticles.setMat4("uView", glm::value_ptr(view));
        m_shaderParticles.setMat4("uProj", glm::value_ptr(proj));
        m_shaderParticles.setFloat("uPointScale", 0.024f * static_cast<float>(m_fbHeight));
        m_shaderParticles.setFloat("uTime", m_simTime);
        glBindVertexArray(m_vaoCloud);
        glDrawArraysInstanced(GL_POINTS, 0, 1, static_cast<GLsizei>(m_cloudCount));
        glDepthMask(GL_TRUE);
    } else {
        m_shaderLine.use();
        m_shaderLine.setMat4("uMVP", glm::value_ptr(vp));
        m_shaderLine.setVec3("uColor", 0.35f, 0.65f, 1.0f);
        glBindVertexArray(m_vaoOrbit);
        glLineWidth(1.5f);
        glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(m_orbitPolyline.size()));

        m_shaderSphere.use();
        glm::mat4 model = glm::translate(glm::mat4(1.0f), m_atom.particles[0].position);
        model = glm::scale(model, glm::vec3(protonScale));
        m_shaderSphere.setMat4("uModel", glm::value_ptr(model));
        m_shaderSphere.setMat4("uView", glm::value_ptr(view));
        m_shaderSphere.setMat4("uProj", glm::value_ptr(proj));
        m_shaderSphere.setVec3("uColor", protonColor.x, protonColor.y, protonColor.z);
        m_shaderSphere.setVec3("uCamPos", camPos.x, camPos.y, camPos.z);
        m_sphereMesh.draw();

        if (m_atom.particles.size() > 1) {
            model = glm::translate(glm::mat4(1.0f), m_atom.particles[1].position);
            model = glm::scale(model, glm::vec3(electronScale));
            m_shaderSphere.setMat4("uModel", glm::value_ptr(model));
            m_shaderSphere.setVec3("uColor", electronColor.x, electronColor.y, electronColor.z);
            m_sphereMesh.draw();
        }
    }

    m_ui.beginFrame();
    m_ui.drawDebugPanel(m_fpsSmooth, static_cast<int>(m_cloudCount), m_qn, m_ql, m_qm,
                        m_quantumMode, camPos, m_timeScale, false, 0.0f);

    std::vector<std::string> consoleVec(m_consoleLines.begin(), m_consoleLines.end());
    m_ui.drawConsole(m_showConsole, consoleVec);
    m_ui.endFrame();
}

void AtomSimulation::onResize(int width, int height) {
    m_fbWidth = width;
    m_fbHeight = height;
    m_ui.resize(width, height);
}

void AtomSimulation::toggleQuantumMode() {
    m_quantumMode = !m_quantumMode;
    std::ostringstream oss;
    oss << "Switched to " << (m_quantumMode ? "Quantum" : "Classical") << " mode";
    addConsoleMessage(oss.str());
}

void AtomSimulation::applyQuantumNumbers() {
    if (m_lastQn == m_qn && m_lastQl == m_ql && m_lastQm == m_qm) {
        return;
    }

    m_sampler.setQuantumNumbers(m_qn, m_ql, m_qm);
    m_needsRegeneration = true;
    m_lastQn = m_qn;
    m_lastQl = m_ql;
    m_lastQm = m_qm;

    std::ostringstream oss;
    oss << "Quantum state: n=" << m_qn << " l=" << m_ql << " m=" << m_qm;
    addConsoleMessage(oss.str());
}

void AtomSimulation::setPreset(int preset) {
    if (preset == 1) {
        m_qn = 1;
        m_ql = 0;
        m_qm = 0;
        m_atom.bohrN = 1;
    } else if (preset == 2) {
        m_qn = 2;
        m_ql = 1;
        m_qm = 0;
        m_atom.bohrN = 2;
    } else if (preset == 3) {
        m_qn = 3;
        m_ql = 2;
        m_qm = 0;
        m_atom.bohrN = 3;
    }
    m_atom.setupHydrogen();
    rebuildOrbit();
    applyQuantumNumbers();
}

void AtomSimulation::decrementN() {
    if (m_qn > 1) {
        --m_qn;
        if (m_ql >= m_qn) {
            m_ql = m_qn - 1;
        }
        if (m_qm > m_ql) {
            m_qm = m_ql;
        }
        if (m_qm < -m_ql) {
            m_qm = -m_ql;
        }
        m_atom.bohrN = m_qn;
        m_atom.setupHydrogen();
        rebuildOrbit();
        applyQuantumNumbers();
    }
}

void AtomSimulation::incrementN() {
    if (m_qn < 8) {
        ++m_qn;
        m_atom.bohrN = m_qn;
        m_atom.setupHydrogen();
        rebuildOrbit();
        applyQuantumNumbers();
    }
}

void AtomSimulation::decrementL() {
    if (m_ql > 0) {
        --m_ql;
        if (m_qm > m_ql) {
            m_qm = m_ql;
        }
        if (m_qm < -m_ql) {
            m_qm = -m_ql;
        }
        applyQuantumNumbers();
    }
}

void AtomSimulation::incrementL() {
    if (m_ql < m_qn - 1) {
        ++m_ql;
        if (m_qm > m_ql) {
            m_qm = m_ql;
        }
        applyQuantumNumbers();
    }
}

void AtomSimulation::decrementM() {
    if (m_qm > -m_ql) {
        --m_qm;
        applyQuantumNumbers();
    }
}

void AtomSimulation::incrementM() {
    if (m_qm < m_ql) {
        ++m_qm;
        applyQuantumNumbers();
    }
}

void AtomSimulation::decreaseParticleCount() {
    if (m_cloudCount > 80000) {
        m_cloudCount -= 25000;
        applyQuantumNumbers();
    }
}

void AtomSimulation::increaseParticleCount() {
    if (m_cloudCount < 650000) {
        m_cloudCount += 25000;
        applyQuantumNumbers();
    }
}

void AtomSimulation::toggleVSync(GLFWwindow*) {
    m_vsyncEnabled = !m_vsyncEnabled;
    glfwSwapInterval(m_vsyncEnabled ? 1 : 0);
    std::ostringstream oss;
    oss << "VSync " << (m_vsyncEnabled ? "enabled" : "disabled");
    addConsoleMessage(oss.str());
}

void AtomSimulation::toggleConsole() {
    m_showConsole = !m_showConsole;
}

void AtomSimulation::adjustTimeScale(float delta) {
    m_timeScale += delta * 0.2f;
    m_timeScale = std::max(0.1f, std::min(10.0f, m_timeScale));

    std::ostringstream oss;
    oss << "Time scale: " << std::fixed << std::setprecision(1) << m_timeScale << "x";
    addConsoleMessage(oss.str());
}
