#ifndef __COBALT_VERTEXATTRIB_H_
#define __COBALT_VERTEXATTRIB_H_

#include "gles.h"

// actual definition of vertexattrib_t is in buffer.h, as they are part of VAO...

void COBALT_API cobalt_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
void COBALT_API cobalt_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
void COBALT_API cobalt_glEnableVertexAttribArray(GLuint index);
void COBALT_API cobalt_glDisableVertexAttribArray(GLuint index);

// other variant are defined in wrap...
void COBALT_API cobalt_glVertexAttrib4f(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void COBALT_API cobalt_glVertexAttrib4fv(GLuint index, const GLfloat *v);


void COBALT_API cobalt_glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params);
void COBALT_API cobalt_glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params);
void COBALT_API cobalt_glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params);
void COBALT_API cobalt_glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid **pointer);

// GL_ARB_instanced_arrays
void COBALT_API cobalt_glVertexAttribDivisor(GLuint index, GLuint divisor);

#endif // __COBALT_VERTEXATTRIB_H_
