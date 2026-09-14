#ifndef __COBALT_COBALT_H_
#define __COBALT_COBALT_H_

#include "khash.h"

#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif // __ARM_NEON__

#include "wrap/gles.h"
#include "gles.h"
#include "glstate.h"

packed_call_t* COBALT_API glCopyPackedCall(const packed_call_t *packed);

#define checkError(code)                          \
    {int error; while ((error = glGetError())) {} \
    code                                          \
    if ((error = glGetError()))                   \
        printf(#code " -> %i\n", error);}

#define printError(file, line)              \
    {int error; if ((error = glGetError())) \
        printf(file ":%i -> %i\n", line, error);}

#define FLUSH_BEGINEND if(glstate->list.pending) cobalt_flush()

#define ERROR_IN_BEGIN if(glstate->list.begin) {errorShim(GL_INVALID_OPERATION); return;}

const GLubyte* COBALT_API cobalt_glGetString(GLenum name);
void COBALT_API cobalt_glGetIntegerv(GLenum pname, GLint *params);
void COBALT_API cobalt_glGetFloatv(GLenum pname, GLfloat *params);
void COBALT_API cobalt_glEnable(GLenum cap);
void COBALT_API cobalt_glDisable(GLenum cap);
void COBALT_API cobalt_glEnableClientState(GLenum cap);
void COBALT_API cobalt_glDisableClientState(GLenum cap);
GLboolean COBALT_API cobalt_glIsEnabled(GLenum cap);
void COBALT_API cobalt_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void COBALT_API cobalt_glDrawArrays(GLenum mode, GLint first, GLsizei count);
void COBALT_API cobalt_glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);
void COBALT_API cobalt_glBegin(GLenum mode);
void COBALT_API cobalt_glEnd(void);
void COBALT_API cobalt_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
void COBALT_API cobalt_glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void COBALT_API cobalt_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void COBALT_API cobalt_glSecondaryColor3f(GLfloat r, GLfloat g, GLfloat b);
void COBALT_API cobalt_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
void COBALT_API cobalt_glMaterialf(GLenum face, GLenum pname, const GLfloat param);
void COBALT_API cobalt_glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void COBALT_API cobalt_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void COBALT_API cobalt_glArrayElement(GLint i);
void COBALT_API cobalt_glLockArrays(GLint first, GLsizei count);
void COBALT_API cobalt_glUnlockArrays(void);
GLuint COBALT_API cobalt_glGenLists(GLsizei range);
void COBALT_API cobalt_glNewList(GLuint list, GLenum mode);
void COBALT_API cobalt_glEndList(void);
void COBALT_API cobalt_glCallList(GLuint list);
void COBALT_API cobalt_glCallLists(GLsizei n, GLenum type, const GLvoid *lists);
void COBALT_API cobalt_glDeleteLists(GLuint list, GLsizei range);
void COBALT_API cobalt_glListBase(GLuint base);
GLboolean COBALT_API cobalt_glIsList(GLuint list);
void COBALT_API cobalt_glPolygonMode(GLenum face, GLenum mode);
GLenum COBALT_API cobalt_glGetError(void);

void COBALT_API cobalt_glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void COBALT_API cobalt_glIndexPointer(GLenum type, GLsizei stride, const GLvoid * pointer);
void COBALT_API cobalt_glEdgeFlagPointer(GLsizei stride, const GLvoid * pointer);
void COBALT_API cobalt_glGetPointerv(GLenum pname, GLvoid* *params);
void COBALT_API cobalt_glFlush(void);
void COBALT_API cobalt_glFinish(void);
void COBALT_API cobalt_glFogfv(GLenum pname, const GLfloat* params);

void COBALT_API cobalt_glStencilMaskSeparate(GLenum face, GLuint mask);

void COBALT_API cobalt_glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
void COBALT_API cobalt_glMultiDrawElements( GLenum mode, GLsizei *count, GLenum type, const void * const *indices, GLsizei primcount);
void COBALT_API cobalt_glMultiDrawElementsBaseVertex( GLenum mode, GLsizei *count, GLenum type, const void * const *indices, GLsizei primcount, const GLint * basevertex);

void COBALT_API cobalt_glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
void COBALT_API cobalt_glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);

void COBALT_API cobalt_glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
void COBALT_API cobalt_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
void COBALT_API cobalt_glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount, GLint basevertex);

const GLubyte* COBALT_API cobalt_glGetStringi(GLenum name, GLuint index);

void COBALT_API cobalt_glClampColor(GLenum target, GLenum clamp);

void cobalt_flush(void);

int adjust_vertices(GLenum mode, int nb);

extern glstate_t *glstate;

void fpe_Init(glstate_t *glstate);       // defined in fpe.c
void fpe_Dispose(glstate_t *glstate);    // defined in fpe.c

// glGetError() return last error, but that error is not reset until read
// So if 2 operations generate an error, 
//  the 2nd error is lost if glGetError has not been called after 1st op
static inline void errorGL() {	// next glGetError will be from GL 
    if(glstate->type_error && glstate->shim_error==GL_NO_ERROR)
	    glstate->type_error = 0;
    else if(glstate->type_error==2)
        glstate->type_error = 1;    // will need to read glGetError...
}
static inline void errorShim(GLenum error) {	// next glGetError will be "error" from cobalt
    if(glstate->type_error && glstate->shim_error==GL_NO_ERROR)
	    glstate->type_error = 1;
    if(glstate->shim_error == GL_NO_ERROR)
	    glstate->shim_error = error;
}
static inline void noerrorShim() {
    if(glstate->type_error && glstate->shim_error==GL_NO_ERROR)
	    glstate->type_error = 1;
}

static inline void noerrorShimNoPurge() {
    // doing nothing
}

void cobalt_scratch(int alloc);
void cobalt_scratch_vertex(int alloc);
void cobalt_scratch_indices(int alloc);
void cobalt_use_scratch_vertex(int use);
void cobalt_use_scratch_indices(int use);

void ToBuffer(int first, int count);
void UnBuffer();

GLboolean COBALT_API glIsList(GLuint list);
GLuint COBALT_API glGenLists(GLsizei range);
void COBALT_API glActiveTextureARB(GLenum texture);
void COBALT_API glArrayElement(GLint i);
void COBALT_API glBegin(GLenum mode);
void COBALT_API glCallList(GLuint list);
void COBALT_API glCallLists(GLsizei n, GLenum type, const GLvoid *lists);
void COBALT_API glClearDepth(GLdouble depth);
void COBALT_API glDeleteList(GLuint list);
void COBALT_API glDeleteLists(GLuint list, GLsizei range);
void COBALT_API glDrawArrays(GLenum mode, GLint first, GLsizei count);
void COBALT_API glEnd(void);
void COBALT_API glEndList(void);
void COBALT_API glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble Near, GLdouble Far);
void COBALT_API glGetDoublev(GLenum pname, GLdouble *params);
void COBALT_API glIndexf(GLfloat i);
void COBALT_API glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);
void COBALT_API glListBase(GLuint base);
void COBALT_API glLockArraysEXT(GLint first, GLsizei count);
void COBALT_API glNewList(GLuint list, GLenum mode);
void COBALT_API glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble Near, GLdouble Far);
void COBALT_API glSecondaryColor3f(GLfloat r, GLfloat g, GLfloat b);
void COBALT_API glTexCoord2f(GLfloat s, GLfloat t);
void COBALT_API glUnlockArraysEXT(void);
void COBALT_API glVertex2f(GLfloat x, GLfloat y);
void COBALT_API glVertex2i(GLint x, GLint y);
void COBALT_API glVertex3f(GLfloat x, GLfloat y, GLfloat z);
GLenum COBALT_API glGetError(void);

// custom functions
void COBALT_API glPushCall(void *call);

#endif // __COBALT_COBALT_H_
