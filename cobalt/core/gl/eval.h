#ifndef __COBALT_EVAL_H_
#define __COBALT_EVAL_H_

#include "const.h"
#include "gles.h"

void COBALT_API cobalt_glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
void COBALT_API cobalt_glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
void COBALT_API cobalt_glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
void COBALT_API cobalt_glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);

void COBALT_API cobalt_glEvalCoord1d(GLdouble u);
void COBALT_API cobalt_glEvalCoord1f(GLfloat u);
void COBALT_API cobalt_glEvalCoord2d(GLdouble u, GLdouble v);
void COBALT_API cobalt_glEvalCoord2f(GLfloat u, GLfloat v);

void COBALT_API cobalt_glEvalMesh1(GLenum mode, GLint i1, GLint i2);
void COBALT_API cobalt_glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
void COBALT_API cobalt_glEvalPoint1(GLint i);
void COBALT_API cobalt_glEvalPoint2(GLint i, GLint j);
void COBALT_API cobalt_glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
void COBALT_API cobalt_glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
void COBALT_API cobalt_glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
void COBALT_API cobalt_glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
void COBALT_API cobalt_glGetMapdv(GLenum target, GLenum query, GLdouble *v);
void COBALT_API cobalt_glGetMapfv(GLenum target, GLenum query, GLfloat *v);
void COBALT_API cobalt_glGetMapiv(GLenum target, GLenum query, GLint *v);

typedef struct {
    GLenum type;
} map_state_t;

typedef struct {
    GLdouble _1, _2, d;
    GLint order;
} mapcoordd_t;

typedef struct {
    GLfloat _1, _2, d;
    GLint order;
} mapcoordf_t;

typedef struct {
    GLenum type;
    GLint dims, width;
    mapcoordd_t u, v;
    const GLdouble *points;
} map_stated_t;

typedef struct {
    GLenum type;
    GLint dims, width;
    mapcoordf_t u, v;
    const GLfloat *points;
} map_statef_t;

typedef struct {
    GLfloat _1, _2;
    GLfloat d;
    GLint n;
} map_grid_t;

static const GLsizei get_map_width(GLenum target) {
    switch (target) {
        case GL_MAP1_COLOR_4:         return 4;
        case GL_MAP1_INDEX:           return 3;
        case GL_MAP1_NORMAL:          return 3;
        case GL_MAP1_TEXTURE_COORD_1: return 1;
        case GL_MAP1_TEXTURE_COORD_2: return 2;
        case GL_MAP1_TEXTURE_COORD_3: return 3;
        case GL_MAP1_TEXTURE_COORD_4: return 4;
        case GL_MAP1_VERTEX_3:        return 3;
        case GL_MAP1_VERTEX_4:        return 4;
        case GL_MAP2_COLOR_4:         return 4;
        case GL_MAP2_INDEX:           return 3;
        case GL_MAP2_NORMAL:          return 3;
        case GL_MAP2_TEXTURE_COORD_1: return 1;
        case GL_MAP2_TEXTURE_COORD_2: return 2;
        case GL_MAP2_TEXTURE_COORD_3: return 3;
        case GL_MAP2_TEXTURE_COORD_4: return 4;
        case GL_MAP2_VERTEX_3:        return 3;
        case GL_MAP2_VERTEX_4:        return 4;
    }
    return 0;
}

#endif // __COBALT_EVAL_H_
