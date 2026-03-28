#include "SimpleUI.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>

SimpleUI::SimpleUI() {}

SimpleUI::~SimpleUI() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vaoText) glDeleteVertexArrays(1, &m_vaoText);
    if (m_vboText) glDeleteBuffers(1, &m_vboText);
    if (m_shaderRect) glDeleteProgram(m_shaderRect);
    if (m_shaderText) glDeleteProgram(m_shaderText);
    if (m_fontTexture) glDeleteTextures(1, &m_fontTexture);
}

GLuint SimpleUI::compileShader(const char* vertSrc, const char* fragSrc) {
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, nullptr);
    glCompileShader(vert);

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, nullptr);
    glCompileShader(frag);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    glDeleteShader(vert);
    glDeleteShader(frag);

    return prog;
}

void SimpleUI::createFontTexture() {
    constexpr int charW = 8;
    constexpr int charH = 8;
    constexpr int gridW = 16;
    constexpr int gridH = 16;
    constexpr int texW = charW * gridW;
    constexpr int texH = charH * gridH;

    std::vector<unsigned char> pixels(texW * texH, 0);

    static const unsigned char font8x8[96][8] = {
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},{0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00},
        {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00},{0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00},
        {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00},{0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00},
        {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00},{0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00},
        {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00},{0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00},
        {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00},{0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00},
        {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06},{0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
        {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00},{0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00},
        {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00},{0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00},
        {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00},{0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00},
        {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00},{0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00},
        {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00},{0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00},
        {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00},{0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00},
        {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00},{0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06},
        {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00},{0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00},
        {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00},{0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00},
        {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00},{0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00},
        {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00},{0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00},
        {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00},{0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00},
        {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00},{0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00},
        {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00},{0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},
        {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00},{0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00},
        {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00},{0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00},
        {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00},{0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00},
        {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00},{0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00},
        {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00},{0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00},
        {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},{0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00},
        {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00},{0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
        {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00},{0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00},
        {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00},{0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00},
        {0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00},{0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00},
        {0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00},{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF},
        {0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00},
        {0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00},{0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00},
        {0x38,0x30,0x30,0x3e,0x33,0x33,0x6E,0x00},{0x00,0x00,0x1E,0x33,0x3f,0x03,0x1E,0x00},
        {0x1C,0x36,0x06,0x0f,0x06,0x06,0x0F,0x00},{0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F},
        {0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00},{0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00},
        {0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E},{0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00},
        {0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00},{0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00},
        {0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00},{0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00},
        {0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F},{0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78},
        {0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00},{0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00},
        {0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00},{0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00},
        {0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00},{0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00},
        {0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00},{0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F},
        {0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00},{0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00},
        {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00},{0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00},
        {0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
    };

    for (int ch = 0; ch < 96; ++ch) {
        int gx = ch % gridW;
        int gy = ch / gridW;
        for (int row = 0; row < 8; ++row) {
            unsigned char byte = font8x8[ch][row];
            for (int col = 0; col < 8; ++col) {
                int px = gx * charW + col;
                int py = gy * charH + row;
                pixels[py * texW + px] = (byte & (1 << col)) ? 255 : 0;
            }
        }
    }

    glGenTextures(1, &m_fontTexture);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texW, texH, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

bool SimpleUI::loadTTFFont(const std::string& fontPath) {
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }
    
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> fontBuffer(static_cast<std::size_t>(size));
    if (!file.read(reinterpret_cast<char*>(fontBuffer.data()), size)) {
        return false;
    }
    
    std::vector<unsigned char> atlasData(m_atlasWidth * m_atlasHeight);
    
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer.data(), 0)) {
        return false;
    }
    
    const float scale = stbtt_ScaleForPixelHeight(&font, m_fontSize);
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    
    int x = 2;
    int y = 2;
    int rowH = 0;
    
    for (int c = 32; c < 128; ++c) {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, c, &advance, &lsb);
        
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &x0, &y0, &x1, &y1);
        
        const int w = x1 - x0;
        const int h = y1 - y0;
        
        if (x + w + 2 >= m_atlasWidth) {
            x = 2;
            y += rowH + 2;
            rowH = 0;
        }
        
        if (y + h + 2 >= m_atlasHeight) {
            return false;
        }
        
        stbtt_MakeCodepointBitmap(&font, atlasData.data() + y * m_atlasWidth + x,
                                 w, h, m_atlasWidth, scale, scale, c);
        
        m_chars[c].x0 = static_cast<float>(x0);
        m_chars[c].y0 = static_cast<float>(y0);
        m_chars[c].x1 = static_cast<float>(x1);
        m_chars[c].y1 = static_cast<float>(y1);
        m_chars[c].u0 = static_cast<float>(x) / m_atlasWidth;
        m_chars[c].v0 = static_cast<float>(y) / m_atlasHeight;
        m_chars[c].u1 = static_cast<float>(x + w) / m_atlasWidth;
        m_chars[c].v1 = static_cast<float>(y + h) / m_atlasHeight;
        m_chars[c].xadvance = advance * scale;
        
        x += w + 2;
        rowH = std::max(rowH, h);
    }
    
    glGenTextures(1, &m_fontTexture);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_atlasWidth, m_atlasHeight, 0, 
                GL_RED, GL_UNSIGNED_BYTE, atlasData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    m_useTTF = true;
    return true;
}

void SimpleUI::init(int screenWidth, int screenHeight, const std::string& fontPath) {
    m_width = screenWidth;
    m_height = screenHeight;
    m_projection = glm::ortho(0.0f, static_cast<float>(m_width), 
                             static_cast<float>(m_height), 0.0f, -1.0f, 1.0f);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glBindVertexArray(0);

    const char* rectVert = R"(
        #version 330 core
        layout(location = 0) in vec4 vertex;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        }
    )";

    const char* rectFrag = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 color;
        void main() {
            FragColor = color;
        }
    )";

    m_shaderRect = compileShader(rectVert, rectFrag);
    
    const char* textVert = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 vTexCoord;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * vec4(aPos, 0.0, 1.0);
            vTexCoord = aTexCoord;
        }
    )";

    const char* textFrag = R"(
        #version 330 core
        in vec2 vTexCoord;
        out vec4 FragColor;
        uniform sampler2D fontTexture;
        uniform vec3 textColor;
        uniform float textAlpha;
        void main() {
            float alpha = texture(fontTexture, vTexCoord).r;
            FragColor = vec4(textColor, alpha * textAlpha);
        }
    )";

    m_shaderText = compileShader(textVert, textFrag);
    
    glGenVertexArrays(1, &m_vaoText);
    glGenBuffers(1, &m_vboText);
    glBindVertexArray(m_vaoText);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboText);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * 256, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                         reinterpret_cast<void*>(2 * sizeof(float)));
    glBindVertexArray(0);
    
    if (!fontPath.empty() && loadTTFFont(fontPath)) {
    } else {
        createFontTexture();
    }
}

void SimpleUI::resize(int screenWidth, int screenHeight) {
    m_width = screenWidth;
    m_height = screenHeight;
    m_projection = glm::ortho(0.0f, static_cast<float>(m_width), 
                             static_cast<float>(m_height), 0.0f, -1.0f, 1.0f);
}

void SimpleUI::beginFrame() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SimpleUI::endFrame() {
    glEnable(GL_DEPTH_TEST);
}

void SimpleUI::drawRect(float x, float y, float w, float h, const glm::vec4& color) {
    float vertices[] = {
        x,     y,     0.0f, 0.0f,
        x + w, y,     1.0f, 0.0f,
        x,     y + h, 0.0f, 1.0f,
        x + w, y,     1.0f, 0.0f,
        x + w, y + h, 1.0f, 1.0f,
        x,     y + h, 0.0f, 1.0f
    };

    glUseProgram(m_shaderRect);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderRect, "projection"), 1, GL_FALSE, 
                      glm::value_ptr(m_projection));
    glUniform4fv(glGetUniformLocation(m_shaderRect, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SimpleUI::drawText(const std::string& text, float x, float y, float scale, 
                       const glm::vec3& color, float alpha) {
    if (text.empty()) return;
    
    std::vector<float> vertices;
    vertices.reserve(text.size() * 24);
    
    float cursorX = x;
    const float startX = x;
    
    if (m_useTTF) {
        for (char c : text) {
            if (c == '\n') {
                cursorX = startX;
                y += m_fontSize * scale * 1.3f;
                continue;
            }
            
            if (c < 32 || c >= 128) {
                c = ' ';
            }
            
            const CharInfo& ch = m_chars[static_cast<int>(c)];
            
            const float x0 = cursorX + ch.x0 * scale;
            const float y0 = y + ch.y0 * scale;
            const float x1 = cursorX + ch.x1 * scale;
            const float y1 = y + ch.y1 * scale;
            
            vertices.insert(vertices.end(), {
                x0, y0, ch.u0, ch.v0,
                x1, y0, ch.u1, ch.v0,
                x0, y1, ch.u0, ch.v1,
                x1, y0, ch.u1, ch.v0,
                x1, y1, ch.u1, ch.v1,
                x0, y1, ch.u0, ch.v1
            });
            
            cursorX += ch.xadvance * scale;
        }
    } else {
        constexpr float charW = 8.0f;
        constexpr float charH = 8.0f;
        constexpr float texW = 128.0f;
        constexpr float texH = 128.0f;
        
        for (char c : text) {
            if (c == '\n') {
                cursorX = startX;
                y += charH * scale * 1.5f;
                continue;
            }
            
            if (c < 32 || c > 126) {
                c = ' ';
            }
            
            const int idx = c - 32;
            const int gx = idx % 16;
            const int gy = idx / 16;
            
            const float u0 = static_cast<float>(gx * 8) / texW;
            const float v0 = static_cast<float>(gy * 8) / texH;
            const float u1 = u0 + charW / texW;
            const float v1 = v0 + charH / texH;
            
            const float x0 = cursorX;
            const float y0 = y;
            const float x1 = cursorX + charW * scale;
            const float y1 = y + charH * scale;
            
            vertices.insert(vertices.end(), {
                x0, y0, u0, v0,
                x1, y0, u1, v0,
                x0, y1, u0, v1,
                x1, y0, u1, v0,
                x1, y1, u1, v1,
                x0, y1, u0, v1
            });
            
            cursorX += charW * scale;
        }
    }
    
    if (vertices.empty()) return;
    
    glUseProgram(m_shaderText);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderText, "projection"), 1, GL_FALSE, 
                      glm::value_ptr(m_projection));
    glUniform3fv(glGetUniformLocation(m_shaderText, "textColor"), 1, glm::value_ptr(color));
    glUniform1f(glGetUniformLocation(m_shaderText, "textAlpha"), alpha);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fontTexture);
    glUniform1i(glGetUniformLocation(m_shaderText, "fontTexture"), 0);
    
    glBindVertexArray(m_vaoText);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboText);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 4));
}

void SimpleUI::drawDebugPanel(float fps, int particleCount, int qn, int ql, int qm,
                             bool quantumMode, const glm::vec3& camPos, float timeScale,
                             bool isGenerating, float genProgress) {
    const float panelX = static_cast<float>(m_width) - 340.0f;
    const float panelY = 10.0f;
    const float panelW = 330.0f;
    const float panelH = isGenerating ? 330.0f : 290.0f;

    drawRect(panelX + 4.0f, panelY + 4.0f, panelW, panelH, glm::vec4(0.0f, 0.0f, 0.0f, 0.3f));
    drawRect(panelX, panelY, panelW, panelH, glm::vec4(0.02f, 0.03f, 0.08f, 0.94f));
    drawRect(panelX, panelY, panelW, 5.0f, glm::vec4(0.3f, 0.6f, 1.0f, 0.95f));
    drawRect(panelX + 2.0f, panelY + 2.0f, panelW - 4.0f, 2.0f, 
            glm::vec4(0.5f, 0.8f, 1.0f, 0.7f));
    drawRect(panelX, panelY + panelH - 1.0f, panelW, 1.0f, glm::vec4(0.3f, 0.6f, 1.0f, 0.5f));

    float textY = panelY + 20.0f;
    const float textX = panelX + 15.0f;
    const float lineSpacing = m_useTTF ? 24.0f : 20.0f;
    
    std::ostringstream oss;
    
    const glm::vec3 fpsColor = (fps > 100.0f) ? glm::vec3(0.3f, 1.0f, 0.4f) :
                               (fps > 60.0f) ? glm::vec3(0.8f, 1.0f, 0.3f) :
                               (fps > 30.0f) ? glm::vec3(1.0f, 0.8f, 0.3f) :
                               glm::vec3(1.0f, 0.3f, 0.3f);
    oss << "FPS: " << static_cast<int>(fps);
    drawText(oss.str(), textX, textY, m_useTTF ? 1.15f : 1.8f, fpsColor);
    textY += lineSpacing;
    
    oss.str("");
    oss << "Particles: " << particleCount;
    drawText(oss.str(), textX, textY, m_useTTF ? 1.0f : 1.5f, glm::vec3(0.9f, 0.9f, 0.9f));
    textY += lineSpacing;
    
    oss.str("");
    oss << "Mode: " << (quantumMode ? "Quantum" : "Classical");
    drawText(oss.str(), textX, textY, m_useTTF ? 1.0f : 1.5f, 
            quantumMode ? glm::vec3(0.4f, 0.8f, 1.0f) : glm::vec3(1.0f, 0.8f, 0.3f));
    textY += lineSpacing + 5.0f;
    
    drawRect(panelX + 10.0f, textY, panelW - 20.0f, 1.0f, glm::vec4(0.3f, 0.6f, 1.0f, 0.3f));
    textY += 8.0f;
    
    drawText("Quantum Numbers:", textX, textY, m_useTTF ? 0.95f : 1.4f, glm::vec3(0.7f, 0.7f, 0.7f));
    textY += lineSpacing;
    
    oss.str("");
    oss << "  n = " << qn;
    drawText(oss.str(), textX, textY, m_useTTF ? 1.0f : 1.5f, glm::vec3(1.0f, 0.6f, 0.3f));
    textY += lineSpacing;
    
    oss.str("");
    oss << "  l = " << ql;
    drawText(oss.str(), textX, textY, m_useTTF ? 1.0f : 1.5f, glm::vec3(0.6f, 1.0f, 0.6f));
    textY += lineSpacing;
    
    oss.str("");
    oss << "  m = " << qm;
    drawText(oss.str(), textX, textY, m_useTTF ? 1.0f : 1.5f, glm::vec3(0.6f, 0.6f, 1.0f));
    textY += lineSpacing + 5.0f;
    
    drawRect(panelX + 10.0f, textY, panelW - 20.0f, 1.0f, glm::vec4(0.3f, 0.6f, 1.0f, 0.3f));
    textY += 8.0f;
    
    drawText("Camera:", textX, textY, m_useTTF ? 0.95f : 1.4f, glm::vec3(0.7f, 0.7f, 0.7f));
    textY += lineSpacing;
    
    oss.str("");
    oss << "  X:" << static_cast<int>(camPos.x) 
        << " Y:" << static_cast<int>(camPos.y) 
        << " Z:" << static_cast<int>(camPos.z);
    drawText(oss.str(), textX, textY, m_useTTF ? 0.9f : 1.3f, glm::vec3(0.8f, 0.8f, 0.8f));
    textY += lineSpacing + 5.0f;
    
    drawRect(panelX + 10.0f, textY, panelW - 20.0f, 1.0f, glm::vec4(0.3f, 0.6f, 1.0f, 0.3f));
    textY += 8.0f;
    
    oss.str("");
    oss << "Time Scale: " << std::fixed << std::setprecision(1) << timeScale << "x";
    const glm::vec3 speedColor = (timeScale > 1.5f) ? glm::vec3(1.0f, 0.4f, 0.3f) : 
                                 (timeScale < 0.8f) ? glm::vec3(0.4f, 0.8f, 1.0f) : 
                                 glm::vec3(0.8f, 0.8f, 0.8f);
    drawText(oss.str(), textX, textY, m_useTTF ? 1.0f : 1.4f, speedColor);
    
    const float scaleBarW = 100.0f;
    const float scaleBarH = 4.0f;
    const float scaleBarX = textX + 150.0f;
    const float scaleBarY = textY + (m_useTTF ? 6.0f : 8.0f);
    const float scaleFill = (timeScale - 0.1f) / 9.9f;
    drawRect(scaleBarX, scaleBarY, scaleBarW, scaleBarH, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));
    drawRect(scaleBarX, scaleBarY, scaleBarW * scaleFill, scaleBarH, glm::vec4(speedColor.x, speedColor.y, speedColor.z, 0.9f));
    
    if (isGenerating) {
        textY += lineSpacing + 5.0f;
        
        oss.str("");
        oss << "Generating... " << static_cast<int>(genProgress * 100.0f) << "%";
        drawText(oss.str(), textX, textY, m_useTTF ? 1.0f : 1.4f, glm::vec3(0.3f, 1.0f, 0.5f));
        textY += lineSpacing;
        
        const float barW = panelW - 30.0f;
        const float barH = 10.0f;
        const float barX = panelX + 15.0f;
        
        drawRect(barX, textY, barW, barH, glm::vec4(0.08f, 0.08f, 0.08f, 0.9f));
        drawRect(barX + 1.0f, textY + 1.0f, (barW - 2.0f) * genProgress, barH - 2.0f, 
                glm::vec4(0.3f, 1.0f, 0.5f, 0.95f));
        drawRect(barX, textY, barW, 1.0f, glm::vec4(0.3f, 1.0f, 0.5f, 0.4f));
    }
}

void SimpleUI::drawConsole(bool visible, const std::vector<std::string>& lines) {
    if (!visible) return;

    const float consoleH = static_cast<float>(m_height) * 0.5f;
    const float consoleY = 10.0f;
    const float consoleW = static_cast<float>(m_width) - 20.0f;

    drawRect(14.0f, consoleY + 4.0f, consoleW, consoleH, glm::vec4(0.0f, 0.0f, 0.0f, 0.4f));
    drawRect(10.0f, consoleY, consoleW, consoleH, glm::vec4(0.01f, 0.02f, 0.05f, 0.97f));
    drawRect(10.0f, consoleY, consoleW, 5.0f, glm::vec4(0.95f, 0.65f, 0.15f, 0.98f));
    drawRect(12.0f, consoleY + 2.0f, consoleW - 4.0f, 2.0f, 
            glm::vec4(1.0f, 0.8f, 0.3f, 0.8f));
    drawRect(10.0f, consoleY + consoleH - 1.0f, consoleW, 1.0f, 
            glm::vec4(0.95f, 0.65f, 0.15f, 0.6f));

    drawText("DEBUG LOG", 20.0f, consoleY + 10.0f, m_useTTF ? 1.0f : 2.2f, 
            glm::vec3(1.0f, 0.85f, 0.3f));
    drawText("[P] to close", consoleW - 140.0f, consoleY + 12.0f, m_useTTF ? 0.9f : 1.4f, 
            glm::vec3(0.7f, 0.7f, 0.7f), 0.8f);

    const float lineHeight = m_useTTF ? (m_fontSize * 1.4f) : 22.0f;
    float textY = consoleY + 45.0f;
    
    const std::size_t maxLines = static_cast<std::size_t>((consoleH - 60.0f) / lineHeight);
    const std::size_t startIdx = (lines.size() > maxLines) ? (lines.size() - maxLines) : 0;
    
    for (std::size_t i = startIdx; i < lines.size(); ++i) {
        const float alpha = 0.5f + 0.5f * static_cast<float>(i - startIdx) / 
                           static_cast<float>(std::max(std::size_t(1), lines.size() - 1));
        drawText(lines[i], 25.0f, textY, m_useTTF ? 0.95f : 1.5f, 
                glm::vec3(0.85f, 0.95f, 1.0f), alpha);
        textY += lineHeight;
    }
}
