#pragma once

#include <string>
#include "GLLoader.h"

class Shader {
public:
    bool compile(const std::string& vertSrc, const std::string& fragSrc);
    void use() const;
    void setFloat(const char* name, float v) const;
    void setInt(const char* name, int v) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setVec4(const char* name, float x, float y, float z, float w) const;
    void setMat4(const char* name, const float* m) const;
    gl::GLuint program() const { return mProgram; }

private:
    gl::GLuint mProgram = 0;
    gl::GLuint compileStage(gl::GLenum type, const std::string& src, bool& ok) const;
};
