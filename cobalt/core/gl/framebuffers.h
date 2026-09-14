#ifndef __COBALT_FRAMEBUFFERS_H_
#define __COBALT_FRAMEBUFFERS_H_

#include "gles.h"

void COBALT_API cobalt_glGenerateMipmap(GLenum target);
void COBALT_API cobalt_glGenFramebuffers(GLsizei n, GLuint *ids);
void COBALT_API cobalt_glDeleteFramebuffers(GLsizei n, GLuint *framebuffers);
GLboolean COBALT_API cobalt_glIsFramebuffer(GLuint framebuffer);
GLenum COBALT_API cobalt_glCheckFramebufferStatus(GLenum target);
void COBALT_API cobalt_glBindFramebuffer(GLenum target, GLuint framebuffer);
void COBALT_API cobalt_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level);
void COBALT_API cobalt_glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
void COBALT_API cobalt_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void COBALT_API cobalt_glDeleteRenderbuffers(GLsizei n, GLuint *renderbuffers);
void COBALT_API cobalt_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void COBALT_API cobalt_glBindRenderbuffer(GLenum target, GLuint renderbuffer);
GLboolean COBALT_API cobalt_glIsRenderbuffer(GLuint renderbuffer);
void COBALT_API cobalt_glGenerateMipmap(GLenum target);
void COBALT_API cobalt_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params);
void COBALT_API cobalt_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint * params);

void COBALT_API cobalt_glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level);   // naive Wrap
void COBALT_API cobalt_glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,	GLint level, GLint layer);   // naive Wrap
void COBALT_API cobalt_glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);    //STUB
void COBALT_API cobalt_glFramebufferTextureLayer(	GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);    // naive Wrap
void COBALT_API cobalt_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

void COBALT_API cobalt_glDrawBuffers(GLsizei n, const GLenum *bufs);
void COBALT_API cobalt_glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum *bufs);

void COBALT_API cobalt_glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint * value);
void COBALT_API cobalt_glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint * value);
void COBALT_API cobalt_glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat * value);
void COBALT_API cobalt_glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);

void COBALT_API cobalt_glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
void COBALT_API cobalt_glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
void COBALT_API cobalt_glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
void COBALT_API cobalt_glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);

void COBALT_API cobalt_glColorMaskIndexed(GLuint framebuffer, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

// In case of COBALT_FB=2, let's create an FBO for everything, that is then blitted just before the SwapBuffer
void createMainFBO(int width, int height);
void blitMainFBO(int x, int y, int width, int height);
void deleteMainFBO(void* state);
void bindMainFBO();
void unbindMainFBO();

void readfboBegin();
void readfboEnd();

GLuint cobalt_getCurrentFBO();
void cobalt_setCurrentFBO();

void cobalt_saveCurrentFBO();
void cobalt_restoreCurrentFBO();

#endif // __COBALT_FRAMEBUFFERS_H_
