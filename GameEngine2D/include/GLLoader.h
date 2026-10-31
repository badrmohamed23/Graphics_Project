#pragma once

#include <cstddef>

#if !defined(APIENTRY)
#if defined(_WIN32)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

namespace gl {
using GLboolean = unsigned char;
using GLbitfield = unsigned int;
using GLint = int;
using GLsizei = int;
using GLuint = unsigned int;
using GLenum = unsigned int;
using GLfloat = float;
using GLchar = char;
using GLubyte = unsigned char;

#undef GL_FALSE
#define GL_FALSE 0
#undef GL_TRUE
#define GL_TRUE 1
#undef GL_TRIANGLES
#define GL_TRIANGLES 0x0004
#undef GL_FLOAT
#define GL_FLOAT 0x1406
#undef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT 0x00004000
#undef GL_BLEND
#define GL_BLEND 0x0BE2
#undef GL_SRC_ALPHA
#define GL_SRC_ALPHA 0x0302
#undef GL_ONE_MINUS_SRC_ALPHA
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#undef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#undef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW 0x88E8
#undef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#undef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#undef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#undef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82

typedef GLenum (APIENTRY* PFN_geterror_t)(void);
typedef void (APIENTRY* PFN_viewport_t)(GLint, GLint, GLsizei, GLsizei);
typedef void (APIENTRY* PFN_clear_t)(GLbitfield);
typedef void (APIENTRY* PFN_clearcolor_t)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY* PFN_blendfunc_t)(GLenum, GLenum);
typedef void (APIENTRY* PFN_enable_t)(GLenum);
typedef void (APIENTRY* PFN_disable_t)(GLenum);
typedef void (APIENTRY* PFN_drawarrays_t)(GLenum, GLint, GLsizei);
typedef void (APIENTRY* PFN_enablevertexattribarray_t)(GLuint);
typedef void (APIENTRY* PFN_vertexattribpointer_t)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void (APIENTRY* PFN_deletebuffers_t)(GLsizei, const GLuint*);
typedef void (APIENTRY* PFN_bufferdata_t)(GLenum, ptrdiff_t, const void*, GLenum);
typedef void (APIENTRY* PFN_bindbuffer_t)(GLenum, GLuint);
typedef void (APIENTRY* PFN_genbuffers_t)(GLsizei, GLuint*);
typedef void (APIENTRY* PFN_deletevertexarrays_t)(GLsizei, const GLuint*);
typedef void (APIENTRY* PFN_bindvertexarray_t)(GLuint);
typedef void (APIENTRY* PFN_genvertexarrays_t)(GLsizei, GLuint*);
typedef void (APIENTRY* PFN_uniformmatrix4fv_t)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void (APIENTRY* PFN_uniform4f_t)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY* PFN_uniform3f_t)(GLint, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY* PFN_uniform1i_t)(GLint, GLint);
typedef void (APIENTRY* PFN_uniform1f_t)(GLint, GLfloat);
typedef GLint (APIENTRY* PFN_getuniformlocation_t)(GLuint, const GLchar*);
typedef void (APIENTRY* PFN_useprogram_t)(GLuint);
typedef void (APIENTRY* PFN_getprograminfolog_t)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (APIENTRY* PFN_getprogramiv_t)(GLuint, GLenum, GLint*);
typedef void (APIENTRY* PFN_linkprogram_t)(GLuint);
typedef void (APIENTRY* PFN_attachshader_t)(GLuint, GLuint);
typedef GLuint (APIENTRY* PFN_createprogram_t)(void);
typedef void (APIENTRY* PFN_getshaderinfolog_t)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void (APIENTRY* PFN_getshaderiv_t)(GLuint, GLenum, GLint*);
typedef void (APIENTRY* PFN_compileshader_t)(GLuint);
typedef void (APIENTRY* PFN_shadersource_t)(GLuint, GLsizei, const GLchar* const*, const GLint*);
typedef const GLubyte* (APIENTRY* PFN_getstring_t)(GLenum);
typedef GLuint (APIENTRY* PFN_createshader_t)(GLenum);

extern PFN_geterror_t glGetError;
extern PFN_viewport_t glViewport;
extern PFN_clear_t glClear;
extern PFN_clearcolor_t glClearColor;
extern PFN_blendfunc_t glBlendFunc;
extern PFN_enable_t glEnable;
extern PFN_disable_t glDisable;
extern PFN_drawarrays_t glDrawArrays;
extern PFN_enablevertexattribarray_t glEnableVertexAttribArray;
extern PFN_vertexattribpointer_t glVertexAttribPointer;
extern PFN_deletebuffers_t glDeleteBuffers;
extern PFN_bufferdata_t glBufferData;
extern PFN_bindbuffer_t glBindBuffer;
extern PFN_genbuffers_t glGenBuffers;
extern PFN_deletevertexarrays_t glDeleteVertexArrays;
extern PFN_bindvertexarray_t glBindVertexArray;
extern PFN_genvertexarrays_t glGenVertexArrays;
extern PFN_uniformmatrix4fv_t glUniformMatrix4fv;
extern PFN_uniform4f_t glUniform4f;
extern PFN_uniform3f_t glUniform3f;
extern PFN_uniform1i_t glUniform1i;
extern PFN_uniform1f_t glUniform1f;
extern PFN_getuniformlocation_t glGetUniformLocation;
extern PFN_useprogram_t glUseProgram;
extern PFN_getprograminfolog_t glGetProgramInfoLog;
extern PFN_getprogramiv_t glGetProgramiv;
extern PFN_linkprogram_t glLinkProgram;
extern PFN_attachshader_t glAttachShader;
extern PFN_createprogram_t glCreateProgram;
extern PFN_getshaderinfolog_t glGetShaderInfoLog;
extern PFN_getshaderiv_t glGetShaderiv;
extern PFN_compileshader_t glCompileShader;
extern PFN_shadersource_t glShaderSource;
extern PFN_getstring_t glGetString;
extern PFN_createshader_t glCreateShader;

using GLProcLoader = void* (*)(const char* name);
bool loadGLFunctions(GLProcLoader loader);
}
