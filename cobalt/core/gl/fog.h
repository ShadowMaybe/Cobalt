#ifndef __COBALT_FOG_H_
#define __COBALT_FOG_H_

#include "gles.h"

typedef struct {
    GLenum          mode;
    GLfloat         density;
    GLenum          distance;
    GLfloat         start;
    GLfloat         end;
    GLfloat         index;
    GLfloat         color[4];
    GLenum          coord_src;
} fog_t;

void COBALT_API cobalt_glFogfv(GLenum pname, const GLfloat* params);
void COBALT_API cobalt_glFogf(GLenum pname, GLfloat param);

void COBALT_API cobalt_glFogCoordf(GLfloat coord);
void COBALT_API cobalt_glFogCoordfv(const GLfloat *coord);

#endif // __COBALT_FOG_H_
