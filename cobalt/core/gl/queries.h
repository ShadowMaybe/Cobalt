#ifndef __COBALT_QUERIES_H_
#define __COBALT_QUERIES_H_

#include "khash.h"
#include "gles.h"

void COBALT_API cobalt_glBeginQuery(GLenum target, GLuint id);
void COBALT_API cobalt_glEndQuery(GLenum target);
void COBALT_API cobalt_glGenQueries(GLsizei n, GLuint * ids);
void COBALT_API cobalt_glDeleteQueries(GLsizei n, const GLuint* ids);
GLboolean COBALT_API cobalt_glIsQuery(GLuint id);
void COBALT_API cobalt_glGetQueryiv(GLenum target, GLenum pname, GLint* params);
void COBALT_API cobalt_glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params);
void COBALT_API cobalt_glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params);
void COBALT_API cobalt_glQueryCounter(GLuint id, GLenum target);
void COBALT_API cobalt_glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 * params);
void COBALT_API cobalt_glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 * params);

unsigned long long get_clock();

#endif // __COBALT_QUERIES_H_
