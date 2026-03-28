#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

class SimpleUI {
public:
    SimpleUI();
    ~SimpleUI();

    void init(int screenWidth, int screenHeight, const std::string& fontPath = "");
    void resize(int screenWidth, int screenHeight);
    void beginFrame();
    void endFrame();
    
    void drawDebugPanel(float fps, int particleCount, int qn, int ql, int qm, 
                       bool quantumMode, const glm::vec3& camPos, float timeScale,
                       bool isGenerating = false, float genProgress = 0.0f);
    void drawConsole(bool visible, const std::vector<std::string>& lines);

private:
    int m_width = 1280;
    int m_height = 720;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_shaderRect = 0;
    GLuint m_shaderText = 0;
    GLuint m_fontTexture = 0;
    GLuint m_vaoText = 0;
    GLuint m_vboText = 0;
    glm::mat4 m_projection{1.0f};
    
    bool m_useTTF = false;
    int m_atlasWidth = 512;
    int m_atlasHeight = 512;
    float m_fontSize = 18.0f;
    
    struct CharInfo {
        float x0, y0, x1, y1;
        float u0, v0, u1, v1;
        float xadvance;
    };
    CharInfo m_chars[128];

    void createFontTexture();
    bool loadTTFFont(const std::string& fontPath);
    void drawRect(float x, float y, float w, float h, const glm::vec4& color);
    void drawText(const std::string& text, float x, float y, float scale, const glm::vec3& color, float alpha = 1.0f);
    
    GLuint compileShader(const char* vertSrc, const char* fragSrc);
};
