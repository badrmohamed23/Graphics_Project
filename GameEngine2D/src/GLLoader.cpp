#include "GLLoader.h"

namespace gl {
PFN_geterror_t glGetError = nullptr;
PFN_viewport_t glViewport = nullptr;
PFN_clear_t glClear = nullptr;
PFN_clearcolor_t glClearColor = nullptr;
PFN_blendfunc_t glBlendFunc = nullptr;
PFN_enable_t glEnable = nullptr;
PFN_disable_t glDisable = nullptr;
PFN_drawarrays_t glDrawArrays = nullptr;
PFN_enablevertexattribarray_t glEnableVertexAttribArray = nullptr;
PFN_vertexattribpointer_t glVertexAttribPointer = nullptr;
PFN_deletebuffers_t glDeleteBuffers = nullptr;
PFN_bufferdata_t glBufferData = nullptr;
PFN_bindbuffer_t glBindBuffer = nullptr;
PFN_genbuffers_t glGenBuffers = nullptr;
PFN_deletevertexarrays_t glDeleteVertexArrays = nullptr;
PFN_bindvertexarray_t glBindVertexArray = nullptr;
PFN_genvertexarrays_t glGenVertexArrays = nullptr;
PFN_uniformmatrix4fv_t glUniformMatrix4fv = nullptr;
PFN_uniform4f_t glUniform4f = nullptr;
PFN_uniform3f_t glUniform3f = nullptr;
PFN_uniform1i_t glUniform1i = nullptr;
PFN_uniform1f_t glUniform1f = nullptr;
PFN_getuniformlocation_t glGetUniformLocation = nullptr;
PFN_useprogram_t glUseProgram = nullptr;
PFN_getprograminfolog_t glGetProgramInfoLog = nullptr;
PFN_getprogramiv_t glGetProgramiv = nullptr;
PFN_linkprogram_t glLinkProgram = nullptr;
PFN_attachshader_t glAttachShader = nullptr;
PFN_createprogram_t glCreateProgram = nullptr;
PFN_getshaderinfolog_t glGetShaderInfoLog = nullptr;
PFN_getshaderiv_t glGetShaderiv = nullptr;
PFN_compileshader_t glCompileShader = nullptr;
PFN_shadersource_t glShaderSource = nullptr;
PFN_getstring_t glGetString = nullptr;
PFN_createshader_t glCreateShader = nullptr;

bool loadGLFunctions(GLProcLoader loader) {
    if (!loader) return false;
    bool ok = true;
    #define GLLOAD(fn, name) { fn = reinterpret_cast<decltype(fn)>(loader(name)); if (!fn) ok = false; }
    GLLOAD(glGetError, "glGetError")
    GLLOAD(glViewport, "glViewport")
    GLLOAD(glClear, "glClear")
    GLLOAD(glClearColor, "glClearColor")
    GLLOAD(glBlendFunc, "glBlendFunc")
    GLLOAD(glEnable, "glEnable")
    GLLOAD(glDisable, "glDisable")
    GLLOAD(glDrawArrays, "glDrawArrays")
    GLLOAD(glEnableVertexAttribArray, "glEnableVertexAttribArray")
    GLLOAD(glVertexAttribPointer, "glVertexAttribPointer")
    GLLOAD(glDeleteBuffers, "glDeleteBuffers")
    GLLOAD(glBufferData, "glBufferData")
    GLLOAD(glBindBuffer, "glBindBuffer")
    GLLOAD(glGenBuffers, "glGenBuffers")
    GLLOAD(glDeleteVertexArrays, "glDeleteVertexArrays")
    GLLOAD(glBindVertexArray, "glBindVertexArray")
    GLLOAD(glGenVertexArrays, "glGenVertexArrays")
    GLLOAD(glUniformMatrix4fv, "glUniformMatrix4fv")
    GLLOAD(glUniform4f, "glUniform4f")
    GLLOAD(glUniform3f, "glUniform3f")
    GLLOAD(glUniform1i, "glUniform1i")
    GLLOAD(glUniform1f, "glUniform1f")
    GLLOAD(glGetUniformLocation, "glGetUniformLocation")
    GLLOAD(glUseProgram, "glUseProgram")
    GLLOAD(glGetProgramInfoLog, "glGetProgramInfoLog")
    GLLOAD(glGetProgramiv, "glGetProgramiv")
    GLLOAD(glLinkProgram, "glLinkProgram")
    GLLOAD(glAttachShader, "glAttachShader")
    GLLOAD(glCreateProgram, "glCreateProgram")
    GLLOAD(glGetShaderInfoLog, "glGetShaderInfoLog")
    GLLOAD(glGetShaderiv, "glGetShaderiv")
    GLLOAD(glCompileShader, "glCompileShader")
    GLLOAD(glShaderSource, "glShaderSource")
    GLLOAD(glGetString, "glGetString")
    GLLOAD(glCreateShader, "glCreateShader")
    #undef GLLOAD
    return ok;
}
}
