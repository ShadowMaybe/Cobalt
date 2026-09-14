#ifndef __COBALT_RASTER_H_
#define __COBALT_RASTER_H_

#include "gles.h"
#include "list.h"

#ifndef GL_STENCIL_INDEX
#define GL_STENCIL_INDEX			0x1901
#endif // GL_STENCIL_INDEX
#ifndef GL_DEPTH_COMPONENT
#define GL_DEPTH_COMPONENT			0x1902
#endif // GL_DEPTH_COMPONENT

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} rasterpos_t;

typedef struct {
    GLint x;
    GLint y;
    GLsizei width;
    GLsizei height;
} viewport_t;

int raster_need_transform();

void COBALT_API cobalt_glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
                     GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
void COBALT_API cobalt_glDrawPixels(GLsizei width, GLsizei height, GLenum format,
                         GLenum type, const GLvoid *data);
void COBALT_API cobalt_glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glWindowPos3f(GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void render_raster();

void COBALT_API cobalt_glPixelZoom(GLfloat xfactor, GLfloat yfactor);

void COBALT_API cobalt_glPixelTransferf(GLenum pname, GLfloat param);

void COBALT_API cobalt_glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values);
void COBALT_API cobalt_glPixelMapuiv(GLenum map,GLsizei mapsize, const GLuint *values);
void COBALT_API cobalt_glPixelMapusv(GLenum map,GLsizei mapsize, const GLushort *values);
void COBALT_API cobalt_glGetPixelMapfv(GLenum map, GLfloat *data);
void COBALT_API cobalt_glGetPixelMapuiv(GLenum map, GLuint *data);
void COBALT_API cobalt_glGetPixelMapusv(GLenum map, GLushort *data);

void render_raster_list(rasterlist_t* raster);

void bitmap_flush();
	
#endif // __COBALT_RASTER_H_
