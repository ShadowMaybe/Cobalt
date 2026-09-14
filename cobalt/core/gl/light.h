#ifndef __COBALT_LIGHT_H_
#define __COBALT_LIGHT_H_

#include "gles.h"
#include "../config.h"

typedef struct {
    GLfloat         ambient[4];
    GLfloat         diffuse[4];
    GLfloat         specular[4];
    GLfloat         constantAttenuation, linearAttenuation, quadraticAttenuation;
    GLfloat         position[4];
    GLfloat         spotDirection[3];  // It's only 3 in fact, because it's a direction
    GLfloat         spotExponent;
    GLfloat         spotCutoff;
} light_t;

typedef struct {
    light_t     lights[MAX_LIGHT];
    GLfloat     ambient[4];
    GLboolean   two_side;
    GLboolean   separate_specular;
    GLboolean   local_viewer;
} light_state_t;

typedef struct {
    GLfloat         ambient[4];
    GLfloat         diffuse[4];
    GLfloat         specular[4];
    GLfloat         emission[4];
    GLfloat         shininess;
    int             indexes[3];
    GLenum          colormat;
} material_t;

typedef struct {
    material_t      front;
    material_t      back;
} material_state_t;

void COBALT_API cobalt_glLightModelf(GLenum pname, GLfloat param);
void COBALT_API cobalt_glLightModelfv(GLenum pname, const GLfloat* params);
void COBALT_API cobalt_glLightfv(GLenum light, GLenum pname, const GLfloat* params);
void COBALT_API cobalt_glLightf(GLenum light, GLenum pname, GLfloat params);

void COBALT_API cobalt_glGetLightfv(GLenum light, GLenum pname, GLfloat * params);
void COBALT_API cobalt_glGetLightiv(GLenum light, GLenum pname, GLint * params);

void COBALT_API cobalt_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params) ;
void COBALT_API cobalt_glMaterialf(GLenum face, GLenum pname, GLfloat param);
void COBALT_API cobalt_glColorMaterial(GLenum face, GLenum mode);
#endif // __COBALT_LIGHT_H_
