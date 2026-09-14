#ifndef __COBALT_BLIT_H_
#define __COBALT_BLIT_H_

#include "gles.h"

#define BLIT_ALPHA      0
#define BLIT_OPAQUE     1
#define BLIT_COLOR      2

void cobalt_blitTexture(GLuint texture, 
    GLfloat sx, GLfloat sy,
    GLfloat width, GLfloat height, 
    GLfloat nwidth, GLfloat nheight, 
    GLfloat zoomx, GLfloat zoomy, 
    GLfloat vpwidth, GLfloat vpheight, 
    GLfloat x, GLfloat y, GLint mode);

#endif // __COBALT_BLIT_H_
