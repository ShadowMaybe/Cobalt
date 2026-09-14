#ifndef GL_WRAP_H
#define GL_WRAP_H

#include "../cobalt.h"
//#include <GLES/glext.h>


// misc naive wrappers

void COBALT_API cobalt_glClearDepth(GLdouble depth);
void COBALT_API cobalt_glClipPlane(GLenum plane, const GLdouble *equation);
void COBALT_API cobalt_glDepthRange(GLdouble nearVal, GLdouble farVal);
void COBALT_API cobalt_glFogi(GLenum pname, GLint param);
void COBALT_API cobalt_glFogiv(GLenum pname, GLint *params);
void COBALT_API cobalt_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble Near, GLdouble Far);
void COBALT_API cobalt_glGetDoublev(GLenum pname, GLdouble *params);
void COBALT_API cobalt_glLighti(GLenum light, GLenum pname, GLint param);
void COBALT_API cobalt_glLightiv(GLenum light, GLenum pname, GLint *iparams);
void COBALT_API cobalt_glLightModeli(GLenum pname, GLint param);
void COBALT_API cobalt_glLightModeliv(GLenum pname, GLint *iparams);
void COBALT_API cobalt_glMateriali(GLenum face, GLenum pname, GLint param);
void COBALT_API cobalt_glMaterialiv(GLenum face, GLenum pname, GLint *param);
void COBALT_API cobalt_glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
void COBALT_API cobalt_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble Near, GLdouble Far);
void COBALT_API cobalt_glGetMaterialiv(GLenum face, GLenum pname, GLint * params);
void COBALT_API cobalt_glGetLightiv(GLenum light, GLenum pname, GLint * params);
void COBALT_API cobalt_glGetClipPlane(GLenum plane, GLdouble *equation);

void COBALT_API cobalt_glDrawRangeElements(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void *indices);	
// color
void COBALT_API cobalt_glColor3f(GLfloat r, GLfloat g, GLfloat b);
void COBALT_API cobalt_glColor3fv(GLfloat *c);
void COBALT_API cobalt_glColor4fv(GLfloat *c);
void COBALT_API cobalt_glIndexfv(const GLfloat *c);
void COBALT_API cobalt_glSecondaryColor3fv(const GLfloat *v);

// raster
void COBALT_API cobalt_glRasterPos2f(GLfloat x, GLfloat y);
void COBALT_API cobalt_glRasterPos2fv(const GLfloat *v);
void COBALT_API cobalt_glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glRasterPos3fv(const GLfloat *v);
void COBALT_API cobalt_glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void COBALT_API cobalt_glRasterPos4fv(const GLfloat *v);
void COBALT_API cobalt_glWindowPos2f(GLfloat x, GLfloat y);
void COBALT_API cobalt_glWindowPos2fv(const GLfloat *v);
void COBALT_API cobalt_glWindowPos3f(GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glWindowPos3fv(const GLfloat *v);

void COBALT_API cobalt_glPixelStoref(GLenum pname, GLfloat param);
void COBALT_API cobalt_glGetTexGendv(GLenum coord,GLenum pname,GLdouble *params);
void COBALT_API cobalt_glGetTexGeniv(GLenum coord,GLenum pname,GLint *params);
void COBALT_API cobalt_glPixelTransferi(GLenum pname, GLint param);
void COBALT_API cobalt_glPixelTransferf(GLenum pname, GLfloat param);	

// eval
void COBALT_API cobalt_glEvalCoord1d(GLdouble u);
void COBALT_API cobalt_glEvalCoord1dv(GLdouble *v);
void COBALT_API cobalt_glEvalCoord1fv(GLfloat *v);
void COBALT_API cobalt_glEvalCoord2d(GLdouble u, GLdouble v);
void COBALT_API cobalt_glEvalCoord2dv(GLdouble *v);
void COBALT_API cobalt_glEvalCoord2fv(GLfloat *v);
void COBALT_API cobalt_glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
void COBALT_API cobalt_glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);

// matrix
void COBALT_API cobalt_glLoadMatrixd(const GLdouble *m);
void COBALT_API cobalt_glMultMatrixd(const GLdouble *m);

// normal
void COBALT_API cobalt_glNormal3fv(GLfloat *v);

// fog (defined in fog.c)
void COBALT_API cobalt_glFogCoordd(GLdouble coord);
void COBALT_API cobalt_glFogCoorddv(const GLdouble *coord);

// rect
#define GL_RECT(suffix, type)                                \
    void COBALT_API cobalt_glRect##suffix(type x1, type y1, type x2, type y2); \
    void COBALT_API cobalt_glRect##suffix##v(const type *v1, const type *v2);

GL_RECT(d, GLdouble)
GL_RECT(f, GLfloat)
GL_RECT(i, GLint)
GL_RECT(s, GLshort)
#undef GL_RECT

// textures

void COBALT_API cobalt_glTexCoord1f(GLfloat s);
void COBALT_API cobalt_glTexCoord1fv(GLfloat *t);
void COBALT_API cobalt_glTexCoord2f(GLfloat s, GLfloat t);
void COBALT_API cobalt_glTexCoord2fv(GLfloat *t);
void COBALT_API cobalt_glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
void COBALT_API cobalt_glTexCoord3fv(GLfloat *t);
void COBALT_API cobalt_glTexCoord4fv(GLfloat *t);

void COBALT_API cobalt_glMultiTexCoord1f(GLenum target, GLfloat s);
void COBALT_API cobalt_glMultiTexCoord1fv(GLenum target, GLfloat *t);
void COBALT_API cobalt_glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
void COBALT_API cobalt_glMultiTexCoord2fv(GLenum target, GLfloat *t);
void COBALT_API cobalt_glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);
void COBALT_API cobalt_glMultiTexCoord3fv(GLenum target, GLfloat *t);
void COBALT_API cobalt_glMultiTexCoord4fv(GLenum target, GLfloat *t);

void COBALT_API cobalt_glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);

void COBALT_API cobalt_glPolygonMode(GLenum face, GLenum mode);

// texgen
void COBALT_API cobalt_glTexGend(GLenum coord, GLenum pname, GLdouble param);
void COBALT_API cobalt_glTexGeni(GLenum coord, GLenum pname, GLint param);
void COBALT_API cobalt_glTexGenf(GLenum coord, GLenum pname, GLfloat param);
void COBALT_API cobalt_glTexGendv(GLenum coord, GLenum pname, const GLdouble *params);
void COBALT_API cobalt_glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
void COBALT_API cobalt_glTexGeniv(GLenum coord, GLenum pname, const GLint *params);

// transforms
void COBALT_API cobalt_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void COBALT_API cobalt_glScaled(GLdouble x, GLdouble y, GLdouble z);
void COBALT_API cobalt_glTranslated(GLdouble x, GLdouble y, GLdouble z);

// vertex
void COBALT_API cobalt_glVertex2f(GLfloat x, GLfloat y);
void COBALT_API cobalt_glVertex2fv(GLfloat *v);
void COBALT_API cobalt_glVertex3fv(GLfloat *v);
void COBALT_API cobalt_glVertex3f(GLfloat r, GLfloat g, GLfloat b);
void COBALT_API cobalt_glVertex4fv(GLfloat *v);

// OES wrappers

void COBALT_API glClearDepthfOES(GLfloat depth);
void COBALT_API glClipPlanefOES(GLenum plane, const GLfloat *equation);
void COBALT_API glDepthRangefOES(GLclampf Near, GLclampf Far);
void COBALT_API glFrustumfOES(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat Near, GLfloat Far);
void COBALT_API glGetClipPlanefOES(GLenum plane, GLfloat equation[4]);
void COBALT_API glOrthofOES(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat Near, GLfloat Far);

// basic thunking

#define THUNK(suffix, type)                                \
void COBALT_API cobalt_glColor3##suffix##v(const type *v);                   \
void COBALT_API cobalt_glColor3##suffix(type r, type g, type b);             \
void COBALT_API cobalt_glColor4##suffix##v(const type *v);                   \
void COBALT_API cobalt_glColor4##suffix(type r, type g, type b, type a);     \
void COBALT_API cobalt_glSecondaryColor3##suffix##v(const type *v);          \
void COBALT_API cobalt_glSecondaryColor3##suffix(type r, type g, type b);    \
void COBALT_API cobalt_glIndex##suffix##v(const type *c);                    \
void COBALT_API cobalt_glIndex##suffix(type c);                              \
void COBALT_API cobalt_glNormal3##suffix##v(const type *v);                  \
void COBALT_API cobalt_glNormal3##suffix(type x, type y, type z);            \
void COBALT_API cobalt_glRasterPos2##suffix##v(type *v);                     \
void COBALT_API cobalt_glRasterPos2##suffix(type x, type y);                 \
void COBALT_API cobalt_glRasterPos3##suffix##v(type *v);                     \
void COBALT_API cobalt_glRasterPos3##suffix(type x, type y, type z);         \
void COBALT_API cobalt_glRasterPos4##suffix##v(type *v);                     \
void COBALT_API cobalt_glRasterPos4##suffix(type x, type y, type z, type w); \
void COBALT_API cobalt_glWindowPos2##suffix##v(type *v);                     \
void COBALT_API cobalt_glWindowPos2##suffix(type x, type y);                 \
void COBALT_API cobalt_glWindowPos3##suffix##v(type *v);                     \
void COBALT_API cobalt_glWindowPos3##suffix(type x, type y, type z);         \
void COBALT_API cobalt_glVertex2##suffix##v(type *v);                        \
void COBALT_API cobalt_glVertex2##suffix(type x, type y);                    \
void COBALT_API cobalt_glVertex3##suffix##v(type *v);                        \
void COBALT_API cobalt_glVertex3##suffix(type x, type y, type z);            \
void COBALT_API cobalt_glVertex4##suffix(type x, type y, type z, type w);    \
void COBALT_API cobalt_glVertex4##suffix##v(type *v);                        \
void COBALT_API cobalt_glTexCoord1##suffix(type s);                          \
void COBALT_API cobalt_glTexCoord1##suffix##v(type *t);                      \
void COBALT_API cobalt_glTexCoord2##suffix(type s, type t);                  \
void COBALT_API cobalt_glTexCoord2##suffix##v(type *t);                      \
void COBALT_API cobalt_glTexCoord3##suffix(type s, type t, type r);          \
void COBALT_API cobalt_glTexCoord3##suffix##v(type *t);                      \
void COBALT_API cobalt_glTexCoord4##suffix(type s, type t, type r, type q);  \
void COBALT_API cobalt_glTexCoord4##suffix##v(type *t);					   \
void COBALT_API cobalt_glMultiTexCoord1##suffix(GLenum target, type s);                          \
void COBALT_API cobalt_glMultiTexCoord1##suffix##v(GLenum target, type *t);                      \
void COBALT_API cobalt_glMultiTexCoord2##suffix(GLenum target, type s, type t);                  \
void COBALT_API cobalt_glMultiTexCoord2##suffix##v(GLenum target, type *t);                      \
void COBALT_API cobalt_glMultiTexCoord3##suffix(GLenum target, type s, type t, type r);          \
void COBALT_API cobalt_glMultiTexCoord3##suffix##v(GLenum target, type *t);                      \
void COBALT_API cobalt_glMultiTexCoord4##suffix(GLenum target, type s, type t, type r, type q);  \
void COBALT_API cobalt_glMultiTexCoord4##suffix##v(GLenum target, type *t);					   \
void COBALT_API cobalt_glMultiTexCoord1##suffix##ARB(GLenum target, type s);                          \
void COBALT_API cobalt_glMultiTexCoord1##suffix##vARB(GLenum target, type *t);                      \
void COBALT_API cobalt_glMultiTexCoord2##suffix##ARB(GLenum target, type s, type t);                  \
void COBALT_API cobalt_glMultiTexCoord2##suffix##vARB(GLenum target, type *t);                      \
void COBALT_API cobalt_glMultiTexCoord3##suffix##ARB(GLenum target, type s, type t, type r);          \
void COBALT_API cobalt_glMultiTexCoord3##suffix##vARB(GLenum target, type *t);                      \
void COBALT_API cobalt_glMultiTexCoord4##suffix##ARB(GLenum target, type s, type t, type r, type q);  \
void COBALT_API cobalt_glMultiTexCoord4##suffix##vARB(GLenum target, type *t);

THUNK(b, GLbyte)
THUNK(d, GLdouble)
THUNK(i, GLint)
THUNK(s, GLshort)
THUNK(ub, GLubyte)
THUNK(ui, GLuint)
THUNK(us, GLushort)
#undef THUNK

#define THUNK(suffix, type) \
void COBALT_API glGet##suffix##v(GLenum pname, type *params);

THUNK(Double, GLdouble)
THUNK(Integer, GLint)
THUNK(Float, GLfloat)
#undef THUNK

#define THUNK(suffix, type) \
void COBALT_API cobalt_glVertexAttrib1##suffix(GLuint index, type v0); \
void COBALT_API cobalt_glVertexAttrib2##suffix(GLuint index, type v0, type v1); \
void COBALT_API cobalt_glVertexAttrib3##suffix(GLuint index, type v0, type v1, type v2); \
void COBALT_API cobalt_glVertexAttrib4##suffix(GLuint index, type v0, type v1, type v2, type v3); \
void COBALT_API cobalt_glVertexAttrib1##suffix##v(GLuint index, const type *v); \
void COBALT_API cobalt_glVertexAttrib2##suffix##v(GLuint index, const type *v); \
void COBALT_API cobalt_glVertexAttrib3##suffix##v(GLuint index, const type *v);
THUNK(s, GLshort);
THUNK(d, GLdouble);
#undef THUNK
void COBALT_API cobalt_glVertexAttrib4dv(GLuint index, const GLdouble *v);
#define THUNK(suffix, type) \
void COBALT_API cobalt_glVertexAttrib4##suffix##v (GLuint index, const type *v); \
void COBALT_API cobalt_glVertexAttrib4N##suffix##v (GLuint index, const type *v)
THUNK(b, GLbyte);
THUNK(ub, GLubyte);
THUNK(s, GLshort);
THUNK(us, GLushort);
THUNK(i, GLint);
THUNK(ui, GLuint);
#undef THUNK
void COBALT_API cobalt_glVertexAttrib4Nub(GLuint index, GLubyte v0, GLubyte v1, GLubyte v2, GLubyte v3);

// ============= GL_ARB_vertex_shader =================
GLvoid COBALT_API glVertexAttrib1fARB(GLuint index, GLfloat v0);
GLvoid COBALT_API glVertexAttrib1sARB(GLuint index, GLshort v0);
GLvoid COBALT_API glVertexAttrib1dARB(GLuint index, GLdouble v0);
GLvoid COBALT_API glVertexAttrib2fARB(GLuint index, GLfloat v0, GLfloat v1);
GLvoid COBALT_API glVertexAttrib2sARB(GLuint index, GLshort v0, GLshort v1);
GLvoid COBALT_API glVertexAttrib2dARB(GLuint index, GLdouble v0, GLdouble v1);
GLvoid COBALT_API glVertexAttrib3fARB(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
GLvoid COBALT_API glVertexAttrib3sARB(GLuint index, GLshort v0, GLshort v1, GLshort v2);
GLvoid COBALT_API glVertexAttrib3dARB(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2);
GLvoid COBALT_API glVertexAttrib4sARB(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3);
GLvoid COBALT_API glVertexAttrib4dARB(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
GLvoid COBALT_API glVertexAttrib4NubARB(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
GLvoid COBALT_API glVertexAttrib1fvARB(GLuint index, const GLfloat *v);
GLvoid COBALT_API glVertexAttrib1svARB(GLuint index, const GLshort *v);
GLvoid COBALT_API glVertexAttrib1dvARB(GLuint index, const GLdouble *v);
GLvoid COBALT_API glVertexAttrib2fvARB(GLuint index, const GLfloat *v);
GLvoid COBALT_API glVertexAttrib2svARB(GLuint index, const GLshort *v);
GLvoid COBALT_API glVertexAttrib2dvARB(GLuint index, const GLdouble *v);
GLvoid COBALT_API glVertexAttrib3fvARB(GLuint index, const GLfloat *v);
GLvoid COBALT_API glVertexAttrib3svARB(GLuint index, const GLshort *v);
GLvoid COBALT_API glVertexAttrib3dvARB(GLuint index, const GLdouble *v);
GLvoid COBALT_API glVertexAttrib4svARB(GLuint index, const GLshort *v);
GLvoid COBALT_API glVertexAttrib4dvARB(GLuint index, const GLdouble *v);
GLvoid COBALT_API glVertexAttrib4ivARB(GLuint index, const GLint *v);
GLvoid COBALT_API glVertexAttrib4bvARB(GLuint index, const GLbyte *v);
GLvoid COBALT_API glVertexAttrib4ubvARB(GLuint index, const GLubyte *v);
GLvoid COBALT_API glVertexAttrib4usvARB(GLuint index, const GLushort *v);
GLvoid COBALT_API glVertexAttrib4uivARB(GLuint index, const GLuint *v);
GLvoid COBALT_API glVertexAttrib4NbvARB(GLuint index, const GLbyte *v);
GLvoid COBALT_API glVertexAttrib4NsvARB(GLuint index, const GLshort *v);
GLvoid COBALT_API glVertexAttrib4NivARB(GLuint index, const GLint *v);
GLvoid COBALT_API glVertexAttrib4NubvARB(GLuint index, const GLubyte *v);
GLvoid COBALT_API glVertexAttrib4NusvARB(GLuint index, const GLushort *v);
GLvoid COBALT_API glVertexAttrib4NuivARB(GLuint index, const GLuint *v);


#endif
