#pragma once

#include <vector>
#include <array>
#include <string>
#include "GLLoader.h"
#include "Shader.h"

struct Vec3 {
    float r = 1.0f, g = 1.0f, b = 1.0f;
    Vec3() = default;
    Vec3(float rr, float gg, float bb) : r(rr), g(gg), b(bb) {}
    Vec3 operator*(float s) const { return Vec3(r * s, g * s, b * s); }
    Vec3 mix(const Vec3& o, float t) const {
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        return Vec3(r + (o.r - r) * t, g + (o.g - g) * t, b + (o.b - b) * t);
    }
    Vec3 grayscale() const {
        float lum = 0.299f * r + 0.587f * g + 0.114f * b;
        return Vec3(lum, lum, lum);
    }
};

class Renderer2D {
public:
    bool init();
    void shutdown();
    void beginFrame(const float* ortho4x4);
    void endFrame();
    void clearFrame(float r, float g, float b) const;
    void setDamageTint(float f);
    void setGrayscale(float f);

    void pushQuad(float cx, float cy, float w, float h, const Vec3& color);
    void pushQuadTL(float x, float y, float w, float h, const Vec3& color);
    void pushTriangle(float ax, float ay, float bx, float by, float cx, float cy, const Vec3& color);
    void pushTri(float cx, float cy, float w, float h, const Vec3& color);
    void pushCircle(float cx, float cy, float radius, const Vec3& color, int segments = 20);
    void pushRectOutline(float x, float y, float w, float h, float thickness, const Vec3& color);
    void pushText(float x, float y, float scale, const std::string& text, const Vec3& color);
    float measureText(float scale, const std::string& text) const;
    void pushBar(float x, float y, float w, float h, float fraction, const Vec3& fg, const Vec3& bg);

private:
    void pushVertex(float x, float y, const Vec3& c);

    static constexpr int FLOATS_PER_VERTEX = 5;
    std::vector<float> mVertices;
    Shader mShader;
    gl::GLuint mVao = 0;
    gl::GLuint mVbo = 0;
    gl::GLint mProjLoc = -1;
    gl::GLint mTintLoc = -1;
    gl::GLint mGrayLoc = -1;
    bool mReady = false;
};
