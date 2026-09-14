#ifndef __COBALT_SHADER_H_
#define __COBALT_SHADER_H_

#include "khash.h"
#include "gles.h"
#include "oldprogram.h"
#include <stdint.h>

typedef struct shaderconv_need_s {
    int         need_color;      // front and back
    int         need_secondary;  //  same
    int         need_fogcoord;
    int         need_texcoord;      // max texcoord needed (-1 for none)
    int         need_notexarray;    // need to not use tex array
    int         need_normalmatrix;  // if normal matrix is needed (for highp / mediump choosing)
    int         need_mvmatrix;
    int         need_mvpmatrix;
    int         need_clean;         // this shader needs to stay "clean", no hack in here
    int         need_clipvertex;
    uint32_t    need_texs;          // flags of what tex is needed
} shaderconv_need_t;

typedef struct shader_s {
    GLuint          id;     // internal id of the shader
    GLenum          type;   // type of the shader (GL_VERTEX or GL_FRAGMENT)
    int             attached; // number of time the shader is attached
    int             deleted;// flagged for deletion
    int             compiled;// flag if compiled
    oldprogram_t   *old;     // in case the shader is an old ARB ASM-like program
    char*           source; // original source of the shader (or converted if coming from "old")
    char*           converted;  // converted source (or null if nothing)
    // shaderconv
    shaderconv_need_t  need;    // the varying need / provide of the shader
} shader_t;

KHASH_MAP_DECLARE_INT(shaderlist, shader_t *);

GLuint COBALT_API cobalt_glCreateShader(GLenum shaderType);
void COBALT_API cobalt_glDeleteShader(GLuint shader);
void COBALT_API cobalt_glCompileShader(GLuint shader);
void COBALT_API cobalt_glShaderSource(GLuint shader, GLsizei count, const GLchar * const *string, const GLint *length);
void COBALT_API cobalt_glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
GLboolean COBALT_API cobalt_glIsShader(GLuint shader);
void COBALT_API cobalt_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
void COBALT_API cobalt_glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
void COBALT_API cobalt_glGetShaderPrecisionFormat(GLenum shaderType, GLenum precisionType, GLint *range, GLint *precision);
void COBALT_API cobalt_glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length);
void COBALT_API cobalt_glReleaseShaderCompiler(void);

void accumShaderNeeds(GLuint shader, shaderconv_need_t *need);
int isShaderCompatible(GLuint shader, shaderconv_need_t *need);
void redoShader(GLuint shader, shaderconv_need_t *need);
shader_t *getShader(GLuint shader);

#define CHECK_SHADER(type, shader) \
    if(!shader) { \
        noerrorShim(); \
        return (type)0; \
    } \
    shader_t *glshader = NULL; \
    khint_t k_##shader; \
    { \
        khash_t(shaderlist) *shaders = glstate->glsl->shaders; \
        k_##shader = kh_get(shaderlist, shaders, shader); \
        if (k_##shader != kh_end(shaders)) \
            glshader = kh_value(shaders, k_##shader); \
    } \
    if (!glshader) { \
        errorShim(GL_INVALID_OPERATION); \
        return (type)0; \
    }

// ========== GL_ARB_shader_objects ==============

GLhandleARB COBALT_API cobalt_glCreateShaderObject(GLenum shaderType);
    
#endif // __COBALT_SHADER_H_
