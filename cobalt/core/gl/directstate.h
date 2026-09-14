#ifndef __COBALT_DIRECTSTATE_H_
#define __COBALT_DIRECTSTATE_H_

#include "gles.h"

void COBALT_API cobalt_glClientAttribDefault(GLbitfield mask);
void COBALT_API cobalt_glPushClientAttribDefault(GLbitfield mask);

void COBALT_API cobalt_glMatrixLoadf(GLenum matrixMode, const GLfloat *m);
void COBALT_API cobalt_glMatrixLoadd(GLenum matrixMode, const GLdouble *m);
void COBALT_API cobalt_glMatrixMultf(GLenum matrixMode, const GLfloat *m);
void COBALT_API cobalt_glMatrixMultd(GLenum matrixMode, const GLdouble *m);
void COBALT_API cobalt_glMatrixLoadIdentity(GLenum matrixMode);
void COBALT_API cobalt_glMatrixRotatef(GLenum matrixMode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glMatrixRotated(GLenum matrixMode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void COBALT_API cobalt_glMatrixScalef(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glMatrixScaled(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z);
void COBALT_API cobalt_glMatrixTranslatef(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glMatrixTranslated(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z);
void COBALT_API cobalt_glMatrixOrtho(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void COBALT_API cobalt_glMatrixFrustum(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void COBALT_API cobalt_glMatrixPush(GLenum matrixMode);
void COBALT_API cobalt_glMatrixPop(GLenum matrixMode);

void COBALT_API cobalt_glTextureParameteri(GLuint texture, GLenum target, GLenum pname, GLint param);
void COBALT_API cobalt_glTextureParameteriv(GLuint texture, GLenum target, GLenum pname, const GLint *param);
void COBALT_API cobalt_glTextureParameterf(GLuint texture, GLenum target, GLenum pname, GLfloat param);
void COBALT_API cobalt_glTextureParameterfv(GLuint texture, GLenum target, GLenum pname, const GLfloat *param);
void COBALT_API cobalt_glTextureImage1D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glTextureImage2D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glCopyTextureImage1D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
void COBALT_API cobalt_glCopyTextureImage2D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void COBALT_API cobalt_glCopyTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void COBALT_API cobalt_glCopyTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void COBALT_API cobalt_glGetTextureImage(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void COBALT_API cobalt_glGetTextureParameterfv(GLuint texture, GLenum target, GLenum pname, GLfloat *params);
void COBALT_API cobalt_glGetTextureParameteriv(GLuint texture, GLenum target, GLenum pname, GLint *params);
void COBALT_API cobalt_glGetTextureLevelParameterfv(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params);
void COBALT_API cobalt_glGetTextureLevelParameteriv(GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params);

void COBALT_API cobalt_glTextureImage3D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glCopyTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

void COBALT_API cobalt_glBindMultiTexture(GLenum texunit, GLenum target, GLuint texture);
void COBALT_API cobalt_glMultiTexCoordPointer(GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void COBALT_API cobalt_glMultiTexEnvf(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
void COBALT_API cobalt_glMultiTexEnvfv(GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
void COBALT_API cobalt_glMultiTexEnvi(GLenum texunit, GLenum target, GLenum pname, GLint param);
void COBALT_API cobalt_glMultiTexEnviv(GLenum texunit, GLenum target, GLenum pname, const GLint *params);
void COBALT_API cobalt_glMultiTexGend(GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
void COBALT_API cobalt_glMultiTexGendv(GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params);
void COBALT_API cobalt_glMultiTexGenf(GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
void COBALT_API cobalt_glMultiTexGenfv(GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params);
void COBALT_API cobalt_glMultiTexGeni(GLenum texunit, GLenum coord, GLenum pname, GLint param);
void COBALT_API cobalt_glMultiTexGeniv(GLenum texunit, GLenum coord, GLenum pname, const GLint *params);
void COBALT_API cobalt_glGetMultiTexEnvfv(GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
void COBALT_API cobalt_glGetMultiTexEnviv(GLenum texunit, GLenum target, GLenum pname, GLint *params);
void COBALT_API cobalt_glGetMultiTexGendv(GLenum texunit, GLenum coord, GLenum pname, GLdouble *params);
void COBALT_API cobalt_glGetMultiTexGenfv(GLenum texunit, GLenum coord, GLenum pname, GLfloat *params);
void COBALT_API cobalt_glGetMultiTexGeniv(GLenum texunit, GLenum coord, GLenum pname, GLint *params);
void COBALT_API cobalt_glMultiTexParameteri(GLenum texunit, GLenum target, GLenum pname, GLint param);
void COBALT_API cobalt_glMultiTexParameteriv(GLenum texunit, GLenum target, GLenum pname, const GLint *param);
void COBALT_API cobalt_glMultiTexParameterf(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
void COBALT_API cobalt_glMultiTexParameterfv(GLenum texunit, GLenum target, GLenum pname, const GLfloat *param);
void COBALT_API cobalt_glMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glCopyMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
void COBALT_API cobalt_glCopyMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void COBALT_API cobalt_glCopyMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void COBALT_API cobalt_glCopyMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void COBALT_API cobalt_glGetMultiTexImage(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void COBALT_API cobalt_glGetMultiTexParameterfv(GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
void COBALT_API cobalt_glGetMultiTexParameteriv(GLenum texunit, GLenum target, GLenum pname, GLint *params);
void COBALT_API cobalt_glGetMultiTexLevelParameterfv(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params);
void COBALT_API cobalt_glGetMultiTexLevelParameteriv(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params);
void COBALT_API cobalt_glMultiTexImage3D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
void COBALT_API cobalt_glCopyMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

void COBALT_API cobalt_glCompressedTextureImage3D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedTextureImage2D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedTextureImage1D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glGetCompressedTextureImage(GLuint texture, GLenum target, GLint level, GLvoid *img);

void COBALT_API cobalt_glCompressedMultiTexImage3D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glCompressedMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
void COBALT_API cobalt_glGetCompressedMultiTexImage(GLenum texunit, GLenum target, GLint level, GLvoid *img);

void COBALT_API cobalt_glMatrixLoadTransposef(GLenum matrixMode, const GLfloat *m);
void COBALT_API cobalt_glMatrixLoadTransposed(GLenum matrixMode, const GLdouble *m);
void COBALT_API cobalt_glMatrixMultTransposef(GLenum matrixMode, const GLfloat *m);
void COBALT_API cobalt_glMatrixMultTransposed(GLenum matrixMode, const GLdouble *m);

void COBALT_API cobalt_glEnableClientStateIndexed(GLenum array, GLuint index);
void COBALT_API cobalt_glDisableClientStateIndexed(GLenum array, GLuint index);
void COBALT_API cobalt_glEnableClientStatei(GLenum array, GLuint index);
void COBALT_API cobalt_glDisableClientStatei(GLenum array, GLuint index);
void COBALT_API cobalt_glEnableVertexArray(GLuint vaobj, GLenum array);
void COBALT_API cobalt_glDisableVertexArray(GLuint vaobj, GLenum array);
void COBALT_API cobalt_glEnableVertexArrayAttrib(GLuint vaobj, GLuint index);
void COBALT_API cobalt_glDisableVertexArrayAttrib(GLuint vaobj, GLuint index);

void COBALT_API cobalt_glGetFloatIndexedv(GLenum target, GLuint index, GLfloat *data);
void COBALT_API cobalt_glGetDoubleIndexedv(GLenum target, GLuint index, GLdouble *data);
void COBALT_API cobalt_glGetIntegerIndexedv(GLenum target, GLuint index, GLint *data);
void COBALT_API cobalt_glGetBooleanIndexedv(GLenum target, GLuint index, GLboolean *data);

void COBALT_API cobalt_glGetPointerIndexedv(GLenum pname, GLuint index, GLvoid **params);

void COBALT_API cobalt_glEnableIndexed(GLenum cap, GLuint index);
void COBALT_API cobalt_glDisableIndexed(GLenum cap, GLuint index);
GLboolean COBALT_API cobalt_glIsEnabledIndexed(GLenum cap, GLuint index);

#endif // __COBALT_DIRECTSTATE_H_
