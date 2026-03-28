#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Atom.hpp"
#include "Camera.hpp"
#include "QuantumHydrogen.hpp"
#include "Shader.hpp"
#include "SimpleUI.hpp"
#include "SphereMesh.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <cmath>
#include <deque>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

struct App {
    GLFWwindow* window = nullptr;
    Camera* camera = nullptr;
    AtomModel atom;
    HydrogenOrbitalSampler sampler{2, 1, 0};
    bool quantumMode = true;
    bool keys[GLFW_KEY_LAST + 1]{};
    int fbWidth = 1280;
    int fbHeight = 720;
    fs::path shaderRoot;
    Shader* shaderLine = nullptr;
    Shader* shaderParticles = nullptr;
    Shader* shaderSphere = nullptr;
    GLuint vaoOrbit = 0;
    GLuint vboOrbit = 0;
    GLuint vaoCloud = 0;
    GLuint vboCloudCorner = 0;
    GLuint vboCloudInst = 0;
    std::vector<glm::vec3> orbitPolyline{};
    std::vector<glm::vec4> cloudPacked{};
    std::uint32_t cloudCount = 360000;
    std::mt19937 rng{std::random_device{}()};
    double lastT = 0.0;
    float simTime = 0.0f;
    int qn = 2;
    int ql = 1;
    int qm = 0;
    bool needsRegeneration = false;
    int lastQn = 2;
    int lastQl = 1;
    int lastQm = 0;
    float bufferUpdateAccum = 0.0f;
    float bufferUpdateInterval = 0.016f;
    bool vsyncEnabled = true;
    SimpleUI* ui = nullptr;
    bool showConsole = false;
    std::deque<std::string> consoleLines;
    std::deque<float> fpsHistory;
    float fpsSmooth = 60.0f;
    bool isGenerating = false;
    std::uint32_t genProgress = 0;
    std::uint32_t genChunkSize = 20000;
    float timeScale = 1.0f;
};

static App* g_app = nullptr;

static void applyQuantumNumbers(App& app);
static void rebuildOrbit(App& app);

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

static void applyQuantumNumbers(App& app) {
    if (app.lastQn == app.qn && app.lastQl == app.ql && app.lastQm == app.qm) {
        return;
    }
    
    app.sampler.setQuantumNumbers(app.qn, app.ql, app.qm);
    app.needsRegeneration = true;
    app.lastQn = app.qn;
    app.lastQl = app.ql;
    app.lastQm = app.qm;
    
    std::ostringstream oss;
    oss << "Quantum state: n=" << app.qn << " l=" << app.ql << " m=" << app.qm;
    app.consoleLines.push_back(oss.str());
    if (app.consoleLines.size() > 20) {
        app.consoleLines.pop_front();
    }
}

static void rebuildOrbit(App& app) {
    AtomModel::fillBohrOrbitPolyline(app.atom.bohrN, app.atom.a0, 220, app.orbitPolyline);
    glBindBuffer(GL_ARRAY_BUFFER, app.vboOrbit);
    const GLsizeiptr bytes =
        static_cast<GLsizeiptr>(app.orbitPolyline.size() * sizeof(glm::vec3));
    glBufferData(GL_ARRAY_BUFFER, bytes, app.orbitPolyline.data(), GL_STATIC_DRAW);
}

static void framebufferSizeCallback(GLFWwindow* /*w*/, int width, int height) {
    if (!g_app) {
        return;
    }
    g_app->fbWidth = width;
    g_app->fbHeight = height;
    glViewport(0, 0, width, height);
    g_app->camera->onResize(width, height);
    if (g_app->ui) {
        g_app->ui->resize(width, height);
    }
}

static void keyCallback(GLFWwindow* w, int key, int /*scancode*/, int action, int /*mods*/) {
    if (!g_app) {
        return;
    }
    if (key < 0 || key > GLFW_KEY_LAST) {
        return;
    }
    if (action == GLFW_PRESS) {
        g_app->keys[key] = true;
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(w, GLFW_TRUE);

        }

        if (key == GLFW_KEY_TAB) {
            g_app->quantumMode = !g_app->quantumMode;
            std::ostringstream oss;
            oss << "Switched to " << (g_app->quantumMode ? "Quantum" : "Classical") << " mode";
            g_app->consoleLines.push_back(oss.str());
            if (g_app->consoleLines.size() > 20) {
                g_app->consoleLines.pop_front();
            }
        }

        if (key == GLFW_KEY_R) {

            applyQuantumNumbers(*g_app);

        }

        if (key == GLFW_KEY_1) {

            g_app->qn = 1;

            g_app->ql = 0;

            g_app->qm = 0;

            g_app->atom.bohrN = 1;

            g_app->atom.setupHydrogen();

            rebuildOrbit(*g_app);

            applyQuantumNumbers(*g_app);

        }

        if (key == GLFW_KEY_2) {

            g_app->qn = 2;

            g_app->ql = 1;

            g_app->qm = 0;

            g_app->atom.bohrN = 2;

            g_app->atom.setupHydrogen();

            rebuildOrbit(*g_app);

            applyQuantumNumbers(*g_app);

        }

        if (key == GLFW_KEY_3) {

            g_app->qn = 3;

            g_app->ql = 2;

            g_app->qm = 0;

            g_app->atom.bohrN = 3;

            g_app->atom.setupHydrogen();

            rebuildOrbit(*g_app);

            applyQuantumNumbers(*g_app);

        }

        if (key == GLFW_KEY_LEFT_BRACKET) {

            if (g_app->qn > 1) {

                --g_app->qn;

                if (g_app->ql >= g_app->qn) {

                    g_app->ql = g_app->qn - 1;

                }

                if (g_app->qm > g_app->ql) {

                    g_app->qm = g_app->ql;

                }

                if (g_app->qm < -g_app->ql) {

                    g_app->qm = -g_app->ql;

                }

                g_app->atom.bohrN = g_app->qn;

                g_app->atom.setupHydrogen();

                rebuildOrbit(*g_app);

                applyQuantumNumbers(*g_app);

            }

        }

        if (key == GLFW_KEY_RIGHT_BRACKET) {

            if (g_app->qn < 8) {

                ++g_app->qn;

                g_app->atom.bohrN = g_app->qn;

                g_app->atom.setupHydrogen();

                rebuildOrbit(*g_app);

                applyQuantumNumbers(*g_app);

            }

        }

        if (key == GLFW_KEY_COMMA) {

            if (g_app->ql > 0) {

                --g_app->ql;

                if (g_app->qm > g_app->ql) {

                    g_app->qm = g_app->ql;

                }

                if (g_app->qm < -g_app->ql) {

                    g_app->qm = -g_app->ql;

                }

                applyQuantumNumbers(*g_app);

            }

        }

        if (key == GLFW_KEY_PERIOD) {

            if (g_app->ql < g_app->qn - 1) {

                ++g_app->ql;

                if (g_app->qm > g_app->ql) {

                    g_app->qm = g_app->ql;

                }

                applyQuantumNumbers(*g_app);

            }

        }

        if (key == GLFW_KEY_SEMICOLON) {

            if (g_app->qm > -g_app->ql) {

                --g_app->qm;

                applyQuantumNumbers(*g_app);

            }

        }

        if (key == GLFW_KEY_APOSTROPHE) {

            if (g_app->qm < g_app->ql) {

                ++g_app->qm;

                applyQuantumNumbers(*g_app);

            }

        }

        if (key == GLFW_KEY_MINUS) {

            if (g_app->cloudCount > 80000) {

                g_app->cloudCount -= 25000;

                applyQuantumNumbers(*g_app);

            }

        }

        if (key == GLFW_KEY_EQUAL) {

            if (g_app->cloudCount < 650000) {

                g_app->cloudCount += 25000;

                applyQuantumNumbers(*g_app);

            }

        }

        if (key == GLFW_KEY_V) {
            g_app->vsyncEnabled = !g_app->vsyncEnabled;
            glfwSwapInterval(g_app->vsyncEnabled ? 1 : 0);
            std::ostringstream oss;
            oss << "VSync " << (g_app->vsyncEnabled ? "enabled" : "disabled");
            g_app->consoleLines.push_back(oss.str());
            if (g_app->consoleLines.size() > 20) {
                g_app->consoleLines.pop_front();
            }
        }

        if (key == GLFW_KEY_P) {
            g_app->showConsole = !g_app->showConsole;
        }

    } else if (action == GLFW_RELEASE) {

        g_app->keys[key] = false;

    }

}



static void cursorPosCallback(GLFWwindow* /*w*/, double x, double y) {
    if (g_app && g_app->camera) {
        g_app->camera->onMouseMoved(x, y);
    }
}

static void scrollCallback(GLFWwindow* /*w*/, double /*xoffset*/, double yoffset) {
    if (!g_app) return;
    
    g_app->timeScale += static_cast<float>(yoffset) * 0.2f;
    g_app->timeScale = std::max(0.1f, std::min(10.0f, g_app->timeScale));
    
    std::ostringstream oss;
    oss << "Time scale: " << std::fixed << std::setprecision(1) << g_app->timeScale << "x";
    g_app->consoleLines.push_back(oss.str());
    if (g_app->consoleLines.size() > 50) {
        g_app->consoleLines.pop_front();
    }
}

static void drawColoredSphere(Shader& sh, const SphereMesh& mesh, const glm::mat4& view,
                              const glm::mat4& proj, const glm::vec3& pos, float scale,
                              const glm::vec3& color, const glm::vec3& camPos) {
    sh.use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
    model = glm::scale(model, glm::vec3(scale));
    sh.setMat4("uModel", glm::value_ptr(model));
    sh.setMat4("uView", glm::value_ptr(view));
    sh.setMat4("uProj", glm::value_ptr(proj));
    sh.setVec3("uColor", color.x, color.y, color.z);
    sh.setVec3("uCamPos", camPos.x, camPos.y, camPos.z);
    mesh.draw();
}



static void initGpuBuffers(App& app) {

    glGenVertexArrays(1, &app.vaoOrbit);

    glGenBuffers(1, &app.vboOrbit);

    glBindVertexArray(app.vaoOrbit);

    glBindBuffer(GL_ARRAY_BUFFER, app.vboOrbit);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4096, nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);



    const glm::vec3 corner(0.0f);

    glGenVertexArrays(1, &app.vaoCloud);

    glGenBuffers(1, &app.vboCloudCorner);

    glGenBuffers(1, &app.vboCloudInst);

    glBindVertexArray(app.vaoCloud);

    glBindBuffer(GL_ARRAY_BUFFER, app.vboCloudCorner);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &corner.x, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

    glEnableVertexAttribArray(0);

    glVertexAttribDivisor(0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, app.vboCloudInst);

    glBufferData(GL_ARRAY_BUFFER,

                 static_cast<GLsizeiptr>(app.cloudPacked.size() * sizeof(glm::vec4)),

                 app.cloudPacked.data(), GL_DYNAMIC_DRAW);

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



int main(int argc, char** argv) {

    App app;

    g_app = &app;

    app.shaderRoot = resolveShaderRoot(argc, argv);



    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



    app.window =

        glfwCreateWindow(app.fbWidth, app.fbHeight, "QuantumAtom", nullptr, nullptr);

    if (!app.window) {

        std::cerr << "glfwCreateWindow failed\n";

        glfwTerminate();

        return 1;

    }

    glfwMakeContextCurrent(app.window);

    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(app.window, framebufferSizeCallback);
    glfwSetKeyCallback(app.window, keyCallback);
    glfwSetCursorPosCallback(app.window, cursorPosCallback);
    glfwSetScrollCallback(app.window, scrollCallback);
    glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {

        std::cerr << "gladLoadGL failed\n";

        glfwTerminate();

        return 1;

    }



    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_PROGRAM_POINT_SIZE);



    Camera camera(app.window, glm::vec3(20.0f, 14.0f, 20.0f), -48.0f, -32.0f);

    app.camera = &camera;

    glfwGetFramebufferSize(app.window, &app.fbWidth, &app.fbHeight);

    framebufferSizeCallback(app.window, app.fbWidth, app.fbHeight);



    const std::string lineVert = (app.shaderRoot / "line.vert").string();

    const std::string lineFrag = (app.shaderRoot / "line.frag").string();

    const std::string particleVert = (app.shaderRoot / "quantum_point.vert").string();

    const std::string particleFrag = (app.shaderRoot / "quantum_point.frag").string();

    const std::string sphereVert = (app.shaderRoot / "sphere.vert").string();

    const std::string sphereFrag = (app.shaderRoot / "sphere.frag").string();



    Shader shLine(lineVert, lineFrag);

    Shader shParticles(particleVert, particleFrag);

    Shader shSphere(sphereVert, sphereFrag);

    SphereMesh meshUnit(1.0f, 14, 14);

    app.shaderLine = &shLine;
    app.shaderParticles = &shParticles;
    app.shaderSphere = &shSphere;

    SimpleUI ui;
    const std::string fontPath = (app.shaderRoot.parent_path() / "fonts" / "Inter-Regular.ttf").string();
    ui.init(app.fbWidth, app.fbHeight, fontPath);
    app.ui = &ui;



    app.atom.bohrN = app.qn;
    app.atom.setupHydrogen();
    app.sampler.setQuantumNumbers(app.qn, app.ql, app.qm);
    app.sampler.generate(app.cloudPacked, app.cloudCount, app.simTime, app.rng);

    initGpuBuffers(app);
    rebuildOrbit(app);

    glBindBuffer(GL_ARRAY_BUFFER, app.vboCloudInst);
    const GLsizeiptr cloudBytes =
        static_cast<GLsizeiptr>(app.cloudPacked.size() * sizeof(glm::vec4));
    glBufferData(GL_ARRAY_BUFFER, cloudBytes, app.cloudPacked.data(), GL_DYNAMIC_DRAW);

    app.lastQn = app.qn;
    app.lastQn = app.qn;
    app.lastQl = app.ql;
    app.lastQm = app.qm;
    app.needsRegeneration = false;
    app.lastT = glfwGetTime();

    app.consoleLines.push_back("=== Quantum Atom Visualizer ===");
    app.consoleLines.push_back("Press P to toggle debug log");
    app.consoleLines.push_back("Controls: TAB=mode, 1/2/3=presets, V=vsync");
    app.consoleLines.push_back("Mouse wheel to adjust time scale");
    app.consoleLines.push_back("System initialized successfully");



    while (!glfwWindowShouldClose(app.window)) {
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - app.lastT);
        app.lastT = now;
        app.simTime += dt;

        const float currentFps = (dt > 0.0f) ? (1.0f / dt) : 60.0f;
        app.fpsHistory.push_back(currentFps);
        if (app.fpsHistory.size() > 30) {
            app.fpsHistory.pop_front();
        }
        float fpsSum = 0.0f;
        for (float f : app.fpsHistory) {
            fpsSum += f;
        }
        app.fpsSmooth = fpsSum / static_cast<float>(app.fpsHistory.size());



        const float scaledDt = dt * app.timeScale;
        
        camera.processFrame(dt, app.keys[GLFW_KEY_W], app.keys[GLFW_KEY_S],
                           app.keys[GLFW_KEY_A], app.keys[GLFW_KEY_D], app.keys[GLFW_KEY_E],
                           app.keys[GLFW_KEY_Q], app.keys[GLFW_KEY_SPACE],
                           app.keys[GLFW_KEY_LEFT_CONTROL]);

        app.atom.updateClassical(scaledDt);



        glClearColor(0.05f, 0.06f, 0.12f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        const glm::mat4 proj = camera.projectionMatrix();

        const glm::mat4 view = camera.viewMatrix();

        const glm::mat4 vp = proj * view;



        const glm::vec3 camPos = camera.position();
        const float protonScale = 0.58f;
        const float electronScale =
            0.17f + 0.035f * std::sqrt(static_cast<float>(app.atom.bohrN));
        const glm::vec3 protonColor(1.0f, 0.92f, 0.2f);
        const glm::vec3 electronColor(0.2f, 0.75f, 1.0f);

        if (app.quantumMode) {
            if (app.needsRegeneration && !app.isGenerating) {
                app.isGenerating = true;
                app.genProgress = 0;
                app.needsRegeneration = false;
            }
            
            if (app.isGenerating) {
                const std::uint32_t remaining = app.cloudCount - app.genProgress;
                const std::uint32_t toGen = std::min(app.genChunkSize, remaining);
                
                app.sampler.generateChunk(app.cloudPacked, app.genProgress, toGen, 
                                        app.cloudCount, app.simTime, app.rng);
                app.genProgress += toGen;
                
                if (app.genProgress >= app.cloudCount) {
                    glBindBuffer(GL_ARRAY_BUFFER, app.vboCloudInst);
                    glBufferData(GL_ARRAY_BUFFER,
                                static_cast<GLsizeiptr>(app.cloudPacked.size() * sizeof(glm::vec4)),
                                app.cloudPacked.data(), GL_DYNAMIC_DRAW);
                    app.isGenerating = false;
                    app.bufferUpdateAccum = 0.0f;
                }
            } else {
                app.bufferUpdateAccum += scaledDt;
                if (app.bufferUpdateAccum >= app.bufferUpdateInterval) {
                    HydrogenOrbitalSampler::stepCloudPositions(app.cloudPacked, app.bufferUpdateAccum, 
                                                              app.qm, app.simTime, app.qn);
                    
                    glBindBuffer(GL_ARRAY_BUFFER, app.vboCloudInst);
                    void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                    if (ptr) {
                        std::memcpy(ptr, app.cloudPacked.data(), app.cloudPacked.size() * sizeof(glm::vec4));
                        glUnmapBuffer(GL_ARRAY_BUFFER);
                    }
                    app.bufferUpdateAccum = 0.0f;
                }
            }

            glDepthMask(GL_FALSE);
            app.shaderParticles->use();
            app.shaderParticles->setMat4("uView", glm::value_ptr(view));
            app.shaderParticles->setMat4("uProj", glm::value_ptr(proj));
            app.shaderParticles->setFloat("uPointScale", 0.024f * static_cast<float>(app.fbHeight));
            app.shaderParticles->setFloat("uTime", app.simTime);
            glBindVertexArray(app.vaoCloud);
            glDrawArraysInstanced(GL_POINTS, 0, 1, static_cast<GLsizei>(app.cloudCount));
            glDepthMask(GL_TRUE);

        } else {

            app.shaderLine->use();

            app.shaderLine->setMat4("uMVP", glm::value_ptr(vp));

            app.shaderLine->setVec3("uColor", 0.35f, 0.65f, 1.0f);

            glBindVertexArray(app.vaoOrbit);

            glLineWidth(1.5f);

            glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(app.orbitPolyline.size()));

            drawColoredSphere(*app.shaderSphere, meshUnit, view, proj,
                              app.atom.particles[0].position, protonScale, protonColor,
                              camPos);
            if (app.atom.particles.size() > 1) {
                drawColoredSphere(*app.shaderSphere, meshUnit, view, proj,
                                  app.atom.particles[1].position, electronScale,
                                  electronColor, camPos);
            }
        }

        app.ui->beginFrame();
        const float genProg = app.isGenerating ? 
            (static_cast<float>(app.genProgress) / static_cast<float>(app.cloudCount)) : 0.0f;
        app.ui->drawDebugPanel(app.fpsSmooth, static_cast<int>(app.cloudCount), 
                              app.qn, app.ql, app.qm, app.quantumMode, camPos, app.timeScale,
                              app.isGenerating, genProg);
        
        std::vector<std::string> consoleVec(app.consoleLines.begin(), app.consoleLines.end());
        app.ui->drawConsole(app.showConsole, consoleVec);
        app.ui->endFrame();

        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }



    glDeleteBuffers(1, &app.vboOrbit);

    glDeleteBuffers(1, &app.vboCloudCorner);

    glDeleteBuffers(1, &app.vboCloudInst);

    glDeleteVertexArrays(1, &app.vaoOrbit);

    glDeleteVertexArrays(1, &app.vaoCloud);



    glfwTerminate();

    return 0;

}

