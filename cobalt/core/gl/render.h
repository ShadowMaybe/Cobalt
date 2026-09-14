#ifndef __COBALT_RENDER_H_
#define __COBALT_RENDER_H_

#include "buffers.h"
#include "gles.h"

GLint COBALT_API cobalt_glRenderMode(GLenum mode);
void COBALT_API cobalt_glInitNames(void);
void COBALT_API cobalt_glPopName(void);
void COBALT_API cobalt_glPushName(GLuint name);
void COBALT_API cobalt_glLoadName(GLuint name);
void COBALT_API cobalt_glSelectBuffer(GLsizei size, GLuint *buffer);

void select_glDrawElements(const vertexattrib_t* vtx, GLenum mode, GLuint count, GLenum type, GLvoid * indices);
void select_glDrawArrays(const vertexattrib_t* vtx, GLenum mode, GLuint first, GLuint count);

#endif // __COBALT_RENDER_H_
