#ifndef __COBALT_TEXGEN_H_
#define __COBALT_TEXGEN_H_

#include "gles.h"

void COBALT_API cobalt_glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
void COBALT_API cobalt_glTexGeni(GLenum coord, GLenum pname, GLint param);
void gen_tex_coords(GLfloat *verts, GLfloat *norm, GLfloat **coords, GLint count, GLint *needclean, int texture, GLushort* indices, GLuint ilen);
void gen_tex_clean(GLint cleancode, int texture);
void COBALT_API cobalt_glGetTexGenfv(GLenum coord,GLenum pname,GLfloat *params);

void COBALT_API cobalt_glLoadTransposeMatrixf(const GLfloat *m);
void COBALT_API cobalt_glLoadTransposeMatrixd(const GLdouble *m);
void COBALT_API cobalt_glMultTransposeMatrixd(const GLdouble *m);
void COBALT_API cobalt_glMultTransposeMatrixf(const GLfloat *m);

#endif // __COBALT_TEXGEN_H_
