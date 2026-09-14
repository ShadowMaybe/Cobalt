#include "directstate.h"

#include <stdio.h>

#include "wrap/cobalt.h"
#include "gles.h"
#include "stack.h"
#include "texgen.h"
#include "debug.h"

//#define DEBUG
#ifdef DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif

// Client State
void COBALT_API cobalt_glClientAttribDefault(GLbitfield mask) {
    if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
        cobalt_glPixelStorei(GL_PACK_ALIGNMENT, 0);
        cobalt_glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
        cobalt_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        cobalt_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        cobalt_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        cobalt_glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        cobalt_glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        cobalt_glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    }
#define enable_disable(pname, enabled)             \
    if (enabled) cobalt_glEnableClientState(pname);       \
    else cobalt_glDisableClientState(pname)

    if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
        int client = glstate->texture.client;
        
        enable_disable(GL_VERTEX_ARRAY, false);
        enable_disable(GL_NORMAL_ARRAY, false);
        enable_disable(GL_COLOR_ARRAY, false);
        enable_disable(GL_SECONDARY_COLOR_ARRAY, false);
        for (int a=0; a<MAX_TEX; a++) {
           cobalt_glClientActiveTexture(GL_TEXTURE0+a);
           enable_disable(GL_TEXTURE_COORD_ARRAY, false);
        }
#undef enable_disable
        if (glstate->texture.client != client) cobalt_glClientActiveTexture(GL_TEXTURE0+client);
    }
}
void COBALT_API cobalt_glPushClientAttribDefault(GLbitfield mask) {
    cobalt_glPushClientAttrib(mask);
    cobalt_glClientAttribDefault(mask);
}

// Matrix
#define mat(f) \
  GLenum old_mat; \
  cobalt_glGetIntegerv(GL_MATRIX_MODE, (GLint *) &old_mat); \
  cobalt_glMatrixMode(matrixMode); \
  cobalt_##f; \
  cobalt_glMatrixMode(old_mat)
  
void COBALT_API cobalt_glMatrixLoadf(GLenum matrixMode, const GLfloat *m) {
    mat(glLoadMatrixf(m));
}
void COBALT_API cobalt_glMatrixLoadd(GLenum matrixMode, const GLdouble *m) {
    mat(glLoadMatrixd(m));
}
void COBALT_API cobalt_glMatrixMultf(GLenum matrixMode, const GLfloat *m) {
    mat(glMultMatrixf(m));
}
void COBALT_API cobalt_glMatrixMultd(GLenum matrixMode, const GLdouble *m) {
        mat(glMultMatrixd(m));
}
void COBALT_API cobalt_glMatrixLoadIdentity(GLenum matrixMode) {
        mat(glLoadIdentity());
}
void COBALT_API cobalt_glMatrixRotatef(GLenum matrixMode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    mat(glRotatef(angle, x, y, z));
}
void COBALT_API cobalt_glMatrixRotated(GLenum matrixMode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
    mat(glRotated(angle, x, y, z));
}
void COBALT_API cobalt_glMatrixScalef(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z) {
    mat(glScalef(x, y, z));
}
void COBALT_API cobalt_glMatrixScaled(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z) {
    mat(glScaled(x, y, z));
}
void COBALT_API cobalt_glMatrixTranslatef(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z) {
    mat(glTranslatef(x, y, z));
}
void COBALT_API cobalt_glMatrixTranslated(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z) {
    mat(glTranslated(x, y, z));
}
void COBALT_API cobalt_glMatrixOrtho(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f) {
    mat(glOrtho(l, r, b ,t, n, f));
}
void COBALT_API cobalt_glMatrixFrustum(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f) {
    mat(glFrustum(l, r, b, t, n, f));
}
void COBALT_API cobalt_glMatrixPush(GLenum matrixMode) {
    mat(glPushMatrix());
}
void COBALT_API cobalt_glMatrixPop(GLenum matrixMode) {
    mat(glPopMatrix());
}
void COBALT_API cobalt_glMatrixLoadTransposef(GLenum matrixMode, const GLfloat *m) {
    mat(glLoadTransposeMatrixf(m));
}
void COBALT_API cobalt_glMatrixLoadTransposed(GLenum matrixMode, const GLdouble *m) {
    mat(glLoadTransposeMatrixd(m));
}
void COBALT_API cobalt_glMatrixMultTransposef(GLenum matrixMode, const GLfloat *m) {
    mat(glMultTransposeMatrixf(m));
}
void COBALT_API cobalt_glMatrixMultTransposed(GLenum matrixMode, const GLdouble *m) {
    mat(glMultTransposeMatrixd(m));
}
#undef mat

// Textures
#define text(f) \
  cobalt_glBindTexture(target, texture); \
  cobalt_##f

void COBALT_API cobalt_glTextureParameteri(GLuint texture, GLenum target, GLenum pname, GLint param) {
    text(glTexParameteri(target, pname, param));
}
void COBALT_API cobalt_glTextureParameteriv(GLuint texture, GLenum target, GLenum pname, const GLint *param) {
    text(glTexParameteriv(target, pname, param));
}
void COBALT_API cobalt_glTextureParameterf(GLuint texture, GLenum target, GLenum pname, GLfloat param) {
    text(glTexParameterf(target, pname, param));
}
void COBALT_API cobalt_glTextureParameterfv(GLuint texture, GLenum target, GLenum pname, const GLfloat *param) {
    text(glTexParameterfv(target, pname, param));
}
void COBALT_API cobalt_glTextureImage1D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexImage1D(target, level, internalformat, width, border, format, type, pixels));
}
void COBALT_API cobalt_glTextureImage2D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels));
}
void COBALT_API cobalt_glTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexSubImage1D(target, level, xoffset, width, format, type, pixels));
}
void COBALT_API cobalt_glTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels));
}
void COBALT_API cobalt_glCopyTextureImage1D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    text(glCopyTexImage1D(target, level, internalformat, x, y, width, border));
}
void COBALT_API cobalt_glCopyTextureImage2D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    text(glCopyTexImage2D(target, level, internalformat, x, y, width, height, border));
}
void COBALT_API cobalt_glCopyTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    text(glCopyTexSubImage1D(target, level, xoffset, x, y, width));
}
void COBALT_API cobalt_glCopyTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    text(glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height));
}
void COBALT_API cobalt_glGetTextureImage(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) {
    text(glGetTexImage(target, level, format, type, pixels));
}
void COBALT_API cobalt_glGetTextureParameterfv(GLuint texture, GLenum target, GLenum pname, GLfloat *params) {
    text(glGetTexParameterfv(target, pname, params));
}
void COBALT_API cobalt_glGetTextureParameteriv(GLuint texture, GLenum target, GLenum pname, GLint *params) {
    text(glGetTexParameteriv(target, pname, params));
}
void COBALT_API cobalt_glGetTextureLevelParameterfv(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params) {
    text(glGetTexLevelParameterfv(target, level, pname, params));
}
void COBALT_API cobalt_glGetTextureLevelParameteriv(GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params) {
    text(glGetTexLevelParameteriv(target, level, pname, params));
}

// Texture 3D
void COBALT_API cobalt_glTextureImage3D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels));
}
void COBALT_API cobalt_glTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels));
}
void COBALT_API cobalt_glCopyTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    text(glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height));
}
// Compressed texture
void COBALT_API cobalt_glCompressedTextureImage3D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data));
}
void COBALT_API cobalt_glCompressedTextureImage2D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data));
}
void COBALT_API cobalt_glCompressedTextureImage1D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data));
}
void COBALT_API cobalt_glCompressedTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data));
}
void COBALT_API cobalt_glCompressedTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data));
}
void COBALT_API cobalt_glCompressedTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data));
}
void COBALT_API cobalt_glGetCompressedTextureImage(GLuint texture, GLenum target, GLint level, GLvoid *img) {
    text(glGetCompressedTexImage(target, level, img));
}

#undef text
#define text(f) \
  GLenum old_tex = glstate->texture.active; \
  if(texunit != old_tex) cobalt_glActiveTexture(texunit); \
  cobalt_##f; \
  if(texunit != old_tex) cobalt_glActiveTexture(old_tex)
#define texc(f) \
  GLenum old_tex = glstate->texture.client; \
  if(texunit != old_tex) cobalt_glClientActiveTexture(texunit); \
  cobalt_##f; \
  if(texunit != old_tex) cobalt_glClientActiveTexture(old_tex)

void COBALT_API cobalt_glBindMultiTexture(GLenum texunit, GLenum target, GLuint texture) {
    text(glBindTexture(target, texture));
}
void COBALT_API cobalt_glMultiTexCoordPointer(GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
    texc(glTexCoordPointer(size, type, stride, pointer));
}
void COBALT_API cobalt_glMultiTexEnvf(GLenum texunit, GLenum target, GLenum pname, GLfloat param) {
    text(glTexEnvf(target, pname, param));
}
void COBALT_API cobalt_glMultiTexEnvfv(GLenum texunit, GLenum target, GLenum pname, const GLfloat *params) {
    text(glTexEnvfv(target, pname, params));
}
void COBALT_API cobalt_glMultiTexEnvi(GLenum texunit, GLenum target, GLenum pname, GLint param) {
    text(glTexEnvi(target, pname, param));
}
void COBALT_API cobalt_glMultiTexEnviv(GLenum texunit, GLenum target, GLenum pname, const GLint *params) {
    text(glTexEnviv(target, pname, params));
}
void COBALT_API cobalt_glMultiTexGend(GLenum texunit, GLenum coord, GLenum pname, GLdouble param) {
    text(glTexGend(coord, pname, param));
}
void COBALT_API cobalt_glMultiTexGendv(GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params) {
    text(glTexGendv(coord, pname, params));
}
void COBALT_API cobalt_glMultiTexGenf(GLenum texunit, GLenum coord, GLenum pname, GLfloat param) {
    text(glTexGenf(coord, pname, param));
}
void COBALT_API cobalt_glMultiTexGenfv(GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params) {
    text(glTexGenfv(coord, pname, params));
}
void COBALT_API cobalt_glMultiTexGeni(GLenum texunit, GLenum coord, GLenum pname, GLint param) {
    text(glTexGeni(coord, pname, param));
}
void COBALT_API cobalt_glMultiTexGeniv(GLenum texunit, GLenum coord, GLenum pname, const GLint *params) {
    text(glTexGeniv(coord, pname, params));
}
void COBALT_API cobalt_glGetMultiTexEnvfv(GLenum texunit, GLenum target, GLenum pname, GLfloat *params) {
    text(glGetTexEnvfv(target, pname, params));
}
void COBALT_API cobalt_glGetMultiTexEnviv(GLenum texunit, GLenum target, GLenum pname, GLint *params) {
    text(glGetTexEnviv(target, pname, params));
}
void COBALT_API cobalt_glGetMultiTexGendv(GLenum texunit, GLenum coord, GLenum pname, GLdouble *params) {
    text(glGetTexGendv(coord, pname, params));
}
void COBALT_API cobalt_glGetMultiTexGenfv(GLenum texunit, GLenum coord, GLenum pname, GLfloat *params) {
    text(glGetTexGenfv(coord, pname, params));
}
void COBALT_API cobalt_glGetMultiTexGeniv(GLenum texunit, GLenum coord, GLenum pname, GLint *params) {
    text(glGetTexGeniv(coord, pname, params));
}
void COBALT_API cobalt_glMultiTexParameteri(GLenum texunit, GLenum target, GLenum pname, GLint param) {
    text(glTexParameteri(target, pname, param));
}
void COBALT_API cobalt_glMultiTexParameteriv(GLenum texunit, GLenum target, GLenum pname, const GLint *param) {
    text(glTexParameteriv(target, pname, param));
}
void COBALT_API cobalt_glMultiTexParameterf(GLenum texunit, GLenum target, GLenum pname, GLfloat param) {
    text(glTexParameterf(target, pname, param));
}
void COBALT_API cobalt_glMultiTexParameterfv(GLenum texunit, GLenum target, GLenum pname, const GLfloat *param) {
    text(glTexParameterfv(target, pname, param));
}
void COBALT_API cobalt_glMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexImage1D(target, level, internalformat, width, border, format, type, pixels));
}
void COBALT_API cobalt_glMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels));
}
void COBALT_API cobalt_glMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexSubImage1D(target, level, xoffset, width, format, type, pixels));
}
void COBALT_API cobalt_glMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels));
}
void COBALT_API cobalt_glCopyMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    text(glCopyTexImage1D(target, level, internalformat, x, y, width, border));
}
void COBALT_API cobalt_glCopyMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    text(glCopyTexImage2D(target, level, internalformat, x, y, width, height, border));
}
void COBALT_API cobalt_glCopyMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    text(glCopyTexSubImage1D(target, level, xoffset, x, y, width));
}
void COBALT_API cobalt_glCopyMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    text(glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height));
}
void COBALT_API cobalt_glGetMultiTexImage(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) {
    text(glGetTexImage(target, level, format, type, pixels));
}
void COBALT_API cobalt_glGetMultiTexParameterfv(GLenum texunit, GLenum target, GLenum pname, GLfloat *params) {
    text(glGetTexParameterfv(target, pname, params));
}
void COBALT_API cobalt_glGetMultiTexParameteriv(GLenum texunit, GLenum target, GLenum pname, GLint *params) {
    text(glGetTexParameteriv(target, pname, params));
}
void COBALT_API cobalt_glGetMultiTexLevelParameterfv(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params) {
    text(glGetTexLevelParameterfv(target, level, pname, params));
}
void COBALT_API cobalt_glGetMultiTexLevelParameteriv(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params) {
    text(glGetTexLevelParameteriv(target, level, pname, params));
}
void COBALT_API cobalt_glMultiTexImage3D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels));
}
void COBALT_API cobalt_glMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels) {
    text(glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels));
}
void COBALT_API cobalt_glCopyMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    text(glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height));
}
// Compressed texture
void COBALT_API cobalt_glCompressedMultiTexImage3D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data));
}
void COBALT_API cobalt_glCompressedMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data));
}
void COBALT_API cobalt_glCompressedMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data));
}
void COBALT_API cobalt_glCompressedMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data));
}
void COBALT_API cobalt_glCompressedMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data));
}
void COBALT_API cobalt_glCompressedMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data) {
    text(glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data));
}
void COBALT_API cobalt_glGetCompressedMultiTexImage(GLenum texunit, GLenum target, GLint level, GLvoid *img) {
    text(glGetCompressedTexImage(target, level, img));
}

void COBALT_API cobalt_glEnableClientStateIndexed(GLenum array, GLuint index) {
    DBG(printf("glEnableClientStateIndexed(%s, %d)\n", PrintEnum(array), index);)
    if (array == GL_TEXTURE_COORD_ARRAY) {
        int old = glstate->texture.client;
        if(old!=index) cobalt_glClientActiveTexture(GL_TEXTURE0+index);
        cobalt_glEnableClientState(array);
        if(old!=index) cobalt_glClientActiveTexture(GL_TEXTURE0+old);
        errorGL();
    } else {
        errorShim(GL_INVALID_ENUM);
    }
}
AliasDecl(void,cobalt_glEnableClientStatei,(GLenum array, GLuint index),cobalt_glEnableClientStateIndexed);

void COBALT_API cobalt_glDisableClientStateIndexed(GLenum array, GLuint index) {
    DBG(printf("glDisableClientStateIndexed(%s, %d)\n", PrintEnum(array), index);)
    if (array == GL_TEXTURE_COORD_ARRAY) {
        int old = glstate->texture.client;
        if(old!=index) cobalt_glClientActiveTexture(GL_TEXTURE0+index);
        cobalt_glDisableClientState(array);
        if(old!=index) cobalt_glClientActiveTexture(GL_TEXTURE0+old);
        errorGL();
    } else {
        errorShim(GL_INVALID_ENUM);
    }
}
AliasDecl(void,cobalt_glDisableClientStatei,(GLenum array, GLuint index),cobalt_glDisableClientStateIndexed);

void COBALT_API cobalt_glEnableVertexArray(GLuint vaobj, GLenum array) {
    DBG(printf("glEnableVertexArray(%d, %s)\n", vaobj, PrintEnum(array));)
    GLuint old = glstate->vao->array;
    cobalt_glBindVertexArray(vaobj);
    cobalt_glEnableClientState(array);
    cobalt_glBindVertexArray(old);
}
void COBALT_API cobalt_glDisableVertexArray(GLuint vaobj, GLenum array) {
    DBG(printf("glDisableVertexArray(%d, %s)\n", vaobj, PrintEnum(array));)
    GLuint old = glstate->vao->array;
    cobalt_glBindVertexArray(vaobj);
    cobalt_glDisableClientState(array);
    cobalt_glBindVertexArray(old);
}
void COBALT_API cobalt_glEnableVertexArrayAttrib(GLuint vaobj, GLuint index) {
    DBG(printf("glEnableVertexArrayAttrib(%d, %d)\n", vaobj, index);)
    GLuint old = glstate->vao->array;
    cobalt_glBindVertexArray(vaobj);
    cobalt_glEnableVertexAttribArray(index);
    cobalt_glBindVertexArray(old);
}
void COBALT_API cobalt_glDisableVertexArrayAttrib(GLuint vaobj, GLuint index) {
    DBG(printf("glDisableVertexArrayAttrib(%d, %d)\n", vaobj, index);)
    GLuint old = glstate->vao->array;
    cobalt_glBindVertexArray(vaobj);
    cobalt_glDisableVertexAttribArray(index);
    cobalt_glBindVertexArray(old);
}

#define GETXXX(XXX, xxx) \
void COBALT_API cobalt_glGet##XXX##Indexedv(GLenum target, GLuint index, GL##xxx *data) { \
    switch(target) { \
     case GL_PROGRAM_MATRIX_EXT: \
     case GL_TRANSPOSE_PROGRAM_MATRIX_EXT: \
     case GL_PROGRAM_MATRIX_STACK_DEPTH_EXT: \
        { \
            int old = glstate->matrix_mode; \
            cobalt_glMatrixMode(GL_MATRIX0_ARB+index); \
            switch(target) { \
             case GL_PROGRAM_MATRIX_EXT: \
                cobalt_glGet##XXX##v(GL_CURRENT_MATRIX_ARB, data); \
                break; \
            case GL_TRANSPOSE_PROGRAM_MATRIX_EXT: \
                cobalt_glGet##XXX##v(GL_TRANSPOSE_CURRENT_MATRIX_ARB, data); \
                break; \
            case GL_PROGRAM_MATRIX_STACK_DEPTH_EXT: \
                cobalt_glGet##XXX##v(GL_CURRENT_MATRIX_STACK_DEPTH_ARB, data); \
                break; \
            } \
            cobalt_glMatrixMode(old); \
        } \
        break; \
        case GL_CURRENT_RASTER_TEXTURE_COORDS: \
        case GL_CURRENT_TEXTURE_COORDS: \
        case GL_TEXTURE_BINDING_1D: \
        case GL_TEXTURE_BINDING_1D_ARRAY: \
        case GL_TEXTURE_BINDING_2D: \
        case GL_TEXTURE_BINDING_2D_ARRAY: \
        case GL_TEXTURE_BINDING_3D: \
        case GL_TEXTURE_BINDING_BUFFER_EXT: \
        case GL_TEXTURE_BINDING_CUBE_MAP: \
        case GL_TEXTURE_BINDING_RECTANGLE_ARB: \
        case GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT: \
        case GL_TEXTURE_BUFFER_FORMAT_EXT: \
        case GL_TEXTURE_GEN_Q: \
        case GL_TEXTURE_GEN_R: \
        case GL_TEXTURE_GEN_S: \
        case GL_TEXTURE_GEN_T: \
        case GL_TEXTURE_MATRIX: \
        case GL_TEXTURE_STACK_DEPTH: \
        case GL_TRANSPOSE_TEXTURE_MATRIX: \
        case GL_TEXTURE_1D: \
        case GL_TEXTURE_2D: \
        case GL_TEXTURE_3D: \
        case GL_TEXTURE_CUBE_MAP: \
        case GL_TEXTURE_RECTANGLE_ARB: \
        { \
            int old = glstate->texture.active; \
            if(old!=index+GL_TEXTURE0) cobalt_glActiveTexture(index+GL_TEXTURE0); \
            cobalt_glGet##XXX##v(target, data); \
            if(old!=index+GL_TEXTURE0) cobalt_glActiveTexture(old); \
        } \
        break; \
        case GL_TEXTURE_COORD_ARRAY: \
        case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING: \
        case GL_TEXTURE_COORD_ARRAY_COUNT: \
        case GL_TEXTURE_COORD_ARRAY_SIZE: \
        case GL_TEXTURE_COORD_ARRAY_STRIDE: \
        case GL_TEXTURE_COORD_ARRAY_TYPE: \
        { \
            int old = glstate->texture.client; \
            if(old!=index) cobalt_glClientActiveTexture(index+GL_TEXTURE0); \
            cobalt_glGet##XXX##v(target, data); \
            if(old!=index) cobalt_glClientActiveTexture(old+GL_TEXTURE0); \
        } \
        break; \
        default: \
            cobalt_glGet##XXX##v(target, data); \
    } \
}

GETXXX(Float, float);
GETXXX(Double, double);
GETXXX(Integer, int);
GETXXX(Boolean, boolean);
#undef GETXXX

void COBALT_API cobalt_glGetPointerIndexedv(GLenum pname, GLuint index, GLvoid **params) {
    int old = glstate->texture.client;
    if(old!=index) cobalt_glClientActiveTexture(index+GL_TEXTURE0);
    cobalt_glGetPointerv(pname, params);
    if(old!=index) cobalt_glClientActiveTexture(old+GL_TEXTURE0);
    
}

void COBALT_API cobalt_glEnableIndexed(GLenum cap, GLuint index) {
    DBG(printf("glEnableIndexed(%s, %d)\n", PrintEnum(cap), index);)
    int old = glstate->texture.active;
    if(old!=index) cobalt_glActiveTexture(index+GL_TEXTURE0);
    cobalt_glEnable(cap);
    if(old!=index) cobalt_glActiveTexture(old);
}

void COBALT_API cobalt_glDisableIndexed(GLenum cap, GLuint index) {
    DBG(printf("glDisableIndexed(%s, %d)\n", PrintEnum(cap), index);)
    int old = glstate->texture.active;
    if(old!=index) cobalt_glActiveTexture(index+GL_TEXTURE0);
    cobalt_glDisable(cap);
    if(old!=index) cobalt_glActiveTexture(old);
}

GLboolean COBALT_API cobalt_glIsEnabledIndexed(GLenum cap, GLuint index) {
    DBG(printf("glIsEnabledIndexed(%s, %d)\n", PrintEnum(cap), index);)
    int old;
    GLboolean rv;
    switch(cap) {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_RECTANGLE_ARB:
        case GL_TEXTURE_GEN_S:
        case GL_TEXTURE_GEN_T:
        case GL_TEXTURE_GEN_R:
        case GL_TEXTURE_GEN_Q:
            old = glstate->texture.active;
            if(old!=index) cobalt_glActiveTexture(index+GL_TEXTURE0);
            rv = cobalt_glIsEnabled(cap);
            if(old!=index) cobalt_glActiveTexture(old);
            return rv;
        case GL_TEXTURE_COORD_ARRAY:
            old = glstate->texture.client;
            if(old!=index) cobalt_glClientActiveTexture(index+GL_TEXTURE0);
            rv = cobalt_glIsEnabled(cap);
            if(old!=index) cobalt_glClientActiveTexture(old+GL_TEXTURE0);
            return rv;
    }
    return cobalt_glIsEnabled(cap);
}

//EXT wrapper
AliasExport(void,glClientAttribDefault,EXT,(GLbitfield mask));
AliasExport(void,glPushClientAttribDefault,EXT,(GLbitfield mask));
AliasExport(void,glMatrixLoadf,EXT,(GLenum matrixMode, const GLfloat *m));
AliasExport(void,glMatrixLoadd,EXT,(GLenum matrixMode, const GLdouble *m));
AliasExport(void,glMatrixMultf,EXT,(GLenum matrixMode, const GLfloat *m));
AliasExport(void,glMatrixMultd,EXT,(GLenum matrixMode, const GLdouble *m));
AliasExport(void,glMatrixLoadIdentity,EXT,(GLenum matrixMode));
AliasExport(void,glMatrixRotatef,EXT,(GLenum matrixMode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z));
AliasExport_D_1(void,glMatrixRotated,EXT,(GLenum matrixMode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z));
AliasExport(void,glMatrixScalef,EXT,(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z));
AliasExport_D_1(void,glMatrixScaled,EXT,(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z));
AliasExport(void,glMatrixTranslatef,EXT,(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z));
AliasExport_D_1(void,glMatrixTranslated,EXT,(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z));
AliasExport_D_1(void,glMatrixOrtho,EXT,(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f));
AliasExport_D_1(void,glMatrixFrustum,EXT,(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f));
AliasExport(void,glMatrixPush,EXT,(GLenum matrixMode));
AliasExport(void,glMatrixPop,EXT,(GLenum matrixMode));
AliasExport(void,glTextureParameteri,EXT,(GLuint texture, GLenum target, GLenum pname, GLint param));
AliasExport(void,glTextureParameteriv,EXT,(GLuint texture, GLenum target, GLenum pname, const GLint *param));
AliasExport(void,glTextureParameterf,EXT,(GLuint texture, GLenum target, GLenum pname, GLfloat param));
AliasExport(void,glTextureParameterfv,EXT,(GLuint texture, GLenum target, GLenum pname, const GLfloat *param));
AliasExport(void,glTextureImage1D,EXT,(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glTextureImage2D,EXT,(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glTextureSubImage1D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glTextureSubImage2D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glCopyTextureImage1D,EXT,(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border));
AliasExport(void,glCopyTextureImage2D,EXT,(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border));
AliasExport(void,glCopyTextureSubImage1D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width));
AliasExport(void,glCopyTextureSubImage2D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height));
AliasExport(void,glGetTextureImage,EXT,(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels));
AliasExport(void,glGetTextureParameterfv,EXT,(GLuint texture, GLenum target, GLenum pname, GLfloat *params));
AliasExport(void,glGetTextureParameteriv,EXT,(GLuint texture, GLenum target, GLenum pname, GLint *params));
AliasExport(void,glGetTextureLevelParameterfv,EXT,(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params));
AliasExport(void,glGetTextureLevelParameteriv,EXT,(GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params));
AliasExport(void,glTextureImage3D,EXT,(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glTextureSubImage3D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glCopyTextureSubImage3D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height));
AliasExport(void,glBindMultiTexture,EXT,(GLenum texunit, GLenum target, GLuint texture));
AliasExport(void,glMultiTexCoordPointer,EXT,(GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer));
AliasExport(void,glMultiTexEnvf,EXT,(GLenum texunit, GLenum target, GLenum pname, GLfloat param));
AliasExport(void,glMultiTexEnvfv,EXT,(GLenum texunit, GLenum target, GLenum pname, const GLfloat *params));
AliasExport(void,glMultiTexEnvi,EXT,(GLenum texunit, GLenum target, GLenum pname, GLint param));
AliasExport(void,glMultiTexEnviv,EXT,(GLenum texunit, GLenum target, GLenum pname, const GLint *params));
AliasExport_M(void,glMultiTexGend,EXT,(GLenum texunit, GLenum coord, GLenum pname, GLdouble param),20);
AliasExport(void,glMultiTexGendv,EXT,(GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params));
AliasExport(void,glMultiTexGenf,EXT,(GLenum texunit, GLenum coord, GLenum pname, GLfloat param));
AliasExport(void,glMultiTexGenfv,EXT,(GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params));
AliasExport(void,glMultiTexGeni,EXT,(GLenum texunit, GLenum coord, GLenum pname, GLint param));
AliasExport(void,glMultiTexGeniv,EXT,(GLenum texunit, GLenum coord, GLenum pname, const GLint *params));
AliasExport(void,glGetMultiTexEnvfv,EXT,(GLenum texunit, GLenum target, GLenum pname, GLfloat *params));
AliasExport(void,glGetMultiTexEnviv,EXT,(GLenum texunit, GLenum target, GLenum pname, GLint *params));
AliasExport(void,glGetMultiTexGendv,EXT,(GLenum texunit, GLenum coord, GLenum pname, GLdouble *params));
AliasExport(void,glGetMultiTexGenfv,EXT,(GLenum texunit, GLenum coord, GLenum pname, GLfloat *params));
AliasExport(void,glGetMultiTexGeniv,EXT,(GLenum texunit, GLenum coord, GLenum pname, GLint *params));
AliasExport(void,glMultiTexParameteri,EXT,(GLenum texunit, GLenum target, GLenum pname, GLint param));
AliasExport(void,glMultiTexParameteriv,EXT,(GLenum texunit, GLenum target, GLenum pname, const GLint *param));
AliasExport(void,glMultiTexParameterf,EXT,(GLenum texunit, GLenum target, GLenum pname, GLfloat param));
AliasExport(void,glMultiTexParameterfv,EXT,(GLenum texunit, GLenum target, GLenum pname, const GLfloat *param));
AliasExport(void,glMultiTexImage1D,EXT,(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glMultiTexImage2D,EXT,(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glMultiTexSubImage1D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glMultiTexSubImage2D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glCopyMultiTexImage1D,EXT,(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border));
AliasExport(void,glCopyMultiTexImage2D,EXT,(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border));
AliasExport(void,glCopyMultiTexSubImage1D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width));
AliasExport(void,glCopyMultiTexSubImage2D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height));
AliasExport(void,glGetMultiTexImage,EXT,(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels));
AliasExport(void,glGetMultiTexParameterfv,EXT,(GLenum texunit, GLenum target, GLenum pname, GLfloat *params));
AliasExport(void,glGetMultiTexParameteriv,EXT,(GLenum texunit, GLenum target, GLenum pname, GLint *params));
AliasExport(void,glGetMultiTexLevelParameterfv,EXT,(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params));
AliasExport(void,glGetMultiTexLevelParameteriv,EXT,(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params));
AliasExport(void,glMultiTexImage3D,EXT,(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glMultiTexSubImage3D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels));
AliasExport(void,glCopyMultiTexSubImage3D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height));
AliasExport(void,glCompressedTextureImage3D,EXT,(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedTextureImage2D,EXT,(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedTextureImage1D,EXT,(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedTextureSubImage3D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedTextureSubImage2D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedTextureSubImage1D,EXT,(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glGetCompressedTextureImage,EXT,(GLuint texture, GLenum target, GLint level, GLvoid *img));
AliasExport(void,glCompressedMultiTexImage3D,EXT,(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedMultiTexImage2D,EXT,(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedMultiTexImage1D,EXT,(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedMultiTexSubImage3D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedMultiTexSubImage2D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glCompressedMultiTexSubImage1D,EXT,(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data));
AliasExport(void,glGetCompressedMultiTexImage,EXT,(GLenum texunit, GLenum target, GLint level, GLvoid *img));
AliasExport(void,glMatrixLoadTransposef,EXT,(GLenum matrixMode, const GLfloat *m));
AliasExport(void,glMatrixLoadTransposed,EXT,(GLenum matrixMode, const GLdouble *m));
AliasExport(void,glMatrixMultTransposef,EXT,(GLenum matrixMode, const GLfloat *m));
AliasExport(void,glMatrixMultTransposed,EXT,(GLenum matrixMode, const GLdouble *m));
AliasExport(void,glEnableClientStateIndexed,EXT,(GLenum array, GLuint index));
AliasExport(void,glDisableClientStateIndexed,EXT,(GLenum array, GLuint index));
AliasExport_A(void,glEnableClientStatei,EXT,(GLenum array, GLuint index),glEnableClientStateIndexed);
AliasExport_A(void,glDisableClientStatei,EXT,(GLenum array, GLuint index),glEnableClientStateIndexed);
AliasExport(void,glEnableVertexArray,EXT,(GLuint vaobj, GLenum array));
AliasExport(void,glDisableVertexArray,EXT,(GLuint vaobj, GLenum array));
AliasExport(void,glEnableVertexArrayAttrib,EXT,(GLuint vaobj, GLuint index));
AliasExport(void,glDisableVertexArrayAttrib,EXT,(GLuint vaobj, GLuint index));
AliasExport(void,glGetPointerIndexedv,EXT,(GLenum pname, GLuint index, GLvoid **params));
AliasExport(void,glGetFloatIndexedv,EXT,(GLenum target, GLuint index, GLfloat *data));
AliasExport(void,glGetDoubleIndexedv,EXT,(GLenum target, GLuint index, GLdouble *data));
AliasExport(void,glGetIntegerIndexedv,EXT,(GLenum target, GLuint index, GLint *data));
AliasExport(void,glGetBooleanIndexedv,EXT,(GLenum target, GLuint index, GLboolean *data));
AliasExport(void,glEnableIndexed,EXT,(GLenum cap, GLuint index));
AliasExport(void,glDisableIndexed,EXT,(GLenum cap, GLuint index));
AliasExport(GLboolean,glIsEnabledIndexed,EXT,(GLenum cap, GLuint index));
#undef text
#undef texc
