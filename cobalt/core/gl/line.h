#ifndef __COBALT_LINE_H
#define __COBALT_LINE_H

#include "gles.h"
#include "list.h"

void COBALT_API cobalt_glLineStipple(GLuint factor, GLushort pattern);
GLfloat *gen_stipple_tex_coords(GLfloat *vert, GLushort *sindices, modeinit_t *modes, int stride, int length, GLfloat* noalloctex);
void bind_stipple_tex();

#endif // __COBALT_LINE_H
