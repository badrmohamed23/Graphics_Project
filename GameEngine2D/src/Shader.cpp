#include "Shader.h"
#include <cstring>

using namespace gl;

GLuint Shader::compileStage(GLenum type, const std::string& src, bool& ok) const {
    fprintf(stderr, "DBG stage %u err_before=%u\n", (unsigned)type, (unsigned)gl::glGetError());
    GLuint s = gl::glCreateShader(type);
    fprintf(stderr, "DBG created=%u err_after_create=%u\n", (unsigned)s, (unsigned)gl::glGetError());
    const GLchar* csrc = src.c_str();
    GLint len = (GLint)src.size();
    gl::glShaderSource(s, 1, &csrc, &len);
    fprintf(stderr, "DBG err_after_source=%u\n", (unsigned)gl::glGetError());
    gl::glCompileShader(s);
    fprintf(stderr, "DBG err_after_compile=%u\n", (unsigned)gl::glGetError());
    GLint status = 0;
    gl::glGetShaderiv(s, GL_COMPILE_STATUS, &status);
    fprintf(stderr, "DBG status=%d err_after_getiv=%u\n", status, (unsigned)gl::glGetError());
    if (!status) {
        char log[1024];
        GLsizei out = 0;
        gl::glGetShaderInfoLog(s, 1024, &out, log);
        fprintf(stderr, "Shader compile error (%s):\n", type == GL_VERTEX_SHADER ? "vert" : "frag");
        fprintf(stderr, "Loghex:");
        for (int debugz = 0; debugz < (int)out && debugz < 64; ++debugz)
            fprintf(stderr, " %02X", (unsigned char)log[debugz]);
        fprintf(stderr, "\nGERR=%u SLEN=%d first=%02X %02X\n", (unsigned)gl::glGetError(), len, (unsigned char)csrc[0], (unsigned char)csrc[1]);
        ok = false;
    }
    return s;
}

bool Shader::compile(const std::string& vertSrc, const std::string& fragSrc) {
    bool ok = true;
    GLuint v = compileStage(GL_VERTEX_SHADER, vertSrc, ok);
    GLuint f = compileStage(GL_FRAGMENT_SHADER, fragSrc, ok);
    if (!ok) return false;
    mProgram = gl::glCreateProgram();
    gl::glAttachShader(mProgram, v);
    gl::glAttachShader(mProgram, f);
    gl::glLinkProgram(mProgram);
    GLint status = 0;
    gl::glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
    if (!status) {
        char log[1024];
        GLsizei out = 0;
        gl::glGetProgramInfoLog(mProgram, 1024, &out, log);
        fprintf(stderr, "Program link error:\n%s\n", log);
        return false;
    }
    return true;
}

void Shader::use() const { gl::glUseProgram(mProgram); }

void Shader::setFloat(const char* name, float v) const {
    gl::glUniform1f(gl::glGetUniformLocation(mProgram, name), v);
}

void Shader::setInt(const char* name, int v) const {
    gl::glUniform1i(gl::glGetUniformLocation(mProgram, name), v);
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    gl::glUniform3f(gl::glGetUniformLocation(mProgram, name), x, y, z);
}

void Shader::setVec4(const char* name, float x, float y, float z, float w) const {
    gl::glUniform4f(gl::glGetUniformLocation(mProgram, name), x, y, z, w);
}

void Shader::setMat4(const char* name, const float* m) const {
    gl::glUniformMatrix4fv(gl::glGetUniformLocation(mProgram, name), 1, GL_FALSE, m);
}

