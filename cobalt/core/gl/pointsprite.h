#ifndef __COBALT_POINTSPRITE_H_
#define __COBALT_POINTSPRITE_H_

#include "gles.h"

typedef struct {
    GLfloat size;
    GLfloat sizeMin;
    GLfloat sizeMax;
    GLfloat fadeThresholdSize;
    GLfloat distance[3];
    GLenum  coordOrigin;
} pointsprite_t;

void COBALT_API cobalt_glPointParameteri(GLenum pname, GLint param);
void COBALT_API cobalt_glPointParameteriv(GLenum pname, const GLint * params);
void COBALT_API cobalt_glPointParameterf(GLenum pname, GLfloat param);
void COBALT_API cobalt_glPointParameterfv(GLenum pname, const GLfloat * params);

void COBALT_API cobalt_glPointSize(GLfloat size);

#endif // __COBALT_POINTSPRITE_H_
