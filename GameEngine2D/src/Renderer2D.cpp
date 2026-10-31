#include "Renderer2D.h"
#include "Config.h"
#include "font8x8_basic.h"

#include <cmath>
#include <cstring>
#include <cstdint>

static const char* SPRITE_VERT = R"(#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 uProj;
out vec3 vColor;
void main() {
    vColor = aColor;
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
}
)";

static const char* SPRITE_FRAG = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;
uniform vec3 uTint;
uniform float uGrayscale;
void main() {
    vec3 c = vColor * uTint;
    float lum = dot(c, vec3(0.299, 0.587, 0.114));
    c = mix(c, vec3(lum), uGrayscale);
    FragColor = vec4(c, 1.0);
}
)";

bool Renderer2D::init() {
    if (!mShader.compile(SPRITE_VERT, SPRITE_FRAG)) return false;
    gl::glGenVertexArrays(1, &mVao);
    gl::glGenBuffers(1, &mVbo);
    gl::glBindVertexArray(mVao);
    gl::glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    gl::glEnableVertexAttribArray(0);
    gl::glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*)0);
    gl::glEnableVertexAttribArray(1);
    gl::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*)(2 * sizeof(float)));
    gl::glBindVertexArray(0);
    mReady = true;
    return true;
}

void Renderer2D::shutdown() {
    if (mVbo) gl::glDeleteBuffers(1, &mVbo);
    if (mVao) gl::glDeleteVertexArrays(1, &mVao);
    mVbo = 0;
    mVao = 0;
    mReady = false;
}

void Renderer2D::beginFrame(const float* ortho4x4) {
    mVertices.clear();
    mShader.use();
    mShader.setMat4("uProj", ortho4x4);
    mShader.setFloat("uGrayscale", 0.0f);
    mShader.setVec3("uTint", 1.0f, 1.0f, 1.0f);
}

void Renderer2D::endFrame() {
    if (mVertices.empty()) return;
    gl::glBindVertexArray(mVao);
    gl::glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    gl::glBufferData(GL_ARRAY_BUFFER, (ptrdiff_t)(mVertices.size() * sizeof(float)), mVertices.data(), GL_DYNAMIC_DRAW);
    gl::glDrawArrays(GL_TRIANGLES, 0, (gl::GLsizei)(mVertices.size() / FLOATS_PER_VERTEX));
    gl::glBindVertexArray(0);
}

void Renderer2D::clearFrame(float r, float g, float b) const {
    gl::glClearColor(r, g, b, 1.0f);
    gl::glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer2D::setDamageTint(float f) {
    f = f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
    mShader.setVec3("uTint", 1.0f, 1.0f - f, 1.0f - f);
}

void Renderer2D::setGrayscale(float f) {
    f = f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
    mShader.setFloat("uGrayscale", f);
}

void Renderer2D::pushVertex(float x, float y, const Vec3& c) {
    mVertices.push_back(x);
    mVertices.push_back(y);
    mVertices.push_back(c.r);
    mVertices.push_back(c.g);
    mVertices.push_back(c.b);
}

void Renderer2D::pushQuad(float cx, float cy, float w, float h, const Vec3& color) {
    float hw = w * 0.5f, hh = h * 0.5f;
    float x0 = cx - hw, x1 = cx + hw, y0 = cy - hh, y1 = cy + hh;
    pushVertex(x0, y0, color); pushVertex(x1, y0, color); pushVertex(x1, y1, color);
    pushVertex(x0, y0, color); pushVertex(x1, y1, color); pushVertex(x0, y1, color);
}

void Renderer2D::pushQuadTL(float x, float y, float w, float h, const Vec3& color) {
    float x1 = x + w, y1 = y + h;
    pushVertex(x, y, color); pushVertex(x1, y, color); pushVertex(x1, y1, color);
    pushVertex(x, y, color); pushVertex(x1, y1, color); pushVertex(x, y1, color);
}

void Renderer2D::pushTri(float cx, float cy, float w, float h, const Vec3& color) {
    float hw = w * 0.5f, hh = h * 0.5f;
    pushVertex(cx - hw, cy - hh, color);
    pushVertex(cx + hw, cy - hh, color);
    pushVertex(cx, cy + hh, color);
}

void Renderer2D::pushCircle(float cx, float cy, float radius, const Vec3& color, int segments) {
    if (segments < 3) segments = 3;
    float prevX = cx + radius, prevY = cy;
    for (int i = 1; i <= segments; ++i) {
        float a = 6.2831853f * (float)i / (float)segments;
        float nx = cx + radius * cosf(a), ny = cy + radius * sinf(a);
        pushVertex(cx, cy, color); pushVertex(prevX, prevY, color); pushVertex(nx, ny, color);
        prevX = nx; prevY = ny;
    }
}

void Renderer2D::pushRectOutline(float x, float y, float w, float h, float t, const Vec3& color) {
    pushQuadTL(x, y, w, t, color);
    pushQuadTL(x, y + h - t, w, t, color);
    pushQuadTL(x, y, t, h, color);
    pushQuadTL(x + w - t, y, t, h, color);
}

float Renderer2D::measureText(float scale, const std::string& text) const {
    return scale * 8.0f * (float)text.size();
}

void Renderer2D::pushText(float x, float y, float scale, const std::string& text, const Vec3& color) {
    float cx = x;
    for (char ch : text) {
        unsigned char uch = (unsigned char)ch;
        if (uch > 127) uch = (unsigned char)'?';
        const char* glyph = font8x8_basic[uch];
        for (int py = 0; py < 8; ++py) {
            for (int px = 0; px < 8; ++px) {
                if (glyph[py] & (1 << px)) {
                    pushQuadTL(cx + px * scale, y + py * scale, scale, scale, color);
                }
            }
        }
        cx += 8.0f * scale;
    }
}

void Renderer2D::pushBar(float x, float y, float w, float h, float fraction, const Vec3& fg, const Vec3& bg) {
    pushQuadTL(x, y, w, h, bg);
    float fw = w * (fraction < 0.0f ? 0.0f : (fraction > 1.0f ? 1.0f : fraction));
    if (fw > 0.5f) pushQuadTL(x, y, fw, h, fg);
}

