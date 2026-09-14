#ifndef __COBALT_SAMPLERS_H_
#define __COBALT_SAMPLERS_H_

#include "gles.h"

void cobalt_glGenSamplers(GLsizei n, GLuint *ids);
void cobalt_glBindSampler(GLuint unit, GLuint sampler);
void cobalt_glDeleteSamplers(GLsizei n, const GLuint* samplers);
GLboolean cobalt_glIsSampler(GLuint id);
void cobalt_glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
void cobalt_glSamplerParameteri(GLuint sampler, GLenum pname, GLint param);
void cobalt_glSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params);
void cobalt_glSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params);
void cobalt_glSamplerParameterIiv(GLuint sampler, GLenum pname, GLint *params);
void cobalt_glSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint *params);
void cobalt_glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat * params);
void cobalt_glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLfloat * params);
void cobalt_glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint * params);
void cobalt_glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint * params);

#endif //__COBALT_SAMPLERS_H_