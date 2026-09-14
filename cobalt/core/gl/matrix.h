#ifndef __COBALT_MATRIX_H_
#define __COBALT_MATRIX_H_

#include "cobalt.h"
#include "gles.h"
#include "glstate.h"
#include "list.h"
#include "matvec.h"

void COBALT_API cobalt_glMatrixMode(GLenum mode);
void COBALT_API cobalt_glPushMatrix(void);
void COBALT_API cobalt_glPopMatrix(void);
void COBALT_API cobalt_glLoadMatrixf(const GLfloat * m);
void COBALT_API cobalt_glMultMatrixf(const GLfloat * m);
void COBALT_API cobalt_glLoadIdentity(void);
void COBALT_API cobalt_glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glScalef(GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void COBALT_API cobalt_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal);
void COBALT_API cobalt_glFrustumf(GLfloat left,	GLfloat right, GLfloat bottom, GLfloat top,	GLfloat nearVal, GLfloat farVal);

static inline GLfloat* getTexMat(int tmu) {
	return glstate->texture_matrix[tmu]->stack+glstate->texture_matrix[tmu]->top*16;
}

static inline GLfloat* getMVMat() {
	return glstate->modelview_matrix->stack+glstate->modelview_matrix->top*16;
}

static inline GLfloat* getInvMVMat() {
	if(glstate->inv_mv_matrix_dirty) {
		matrix_inverse(glstate->modelview_matrix->stack+glstate->modelview_matrix->top*16, glstate->inv_mv_matrix);
		glstate->inv_mv_matrix_dirty = 0;
	}
	return glstate->inv_mv_matrix;
}
static inline GLfloat* getNormalMat() {
	if(glstate->normal_matrix_dirty) {
		matrix_inverse3_transpose(glstate->modelview_matrix->stack+glstate->modelview_matrix->top*16, glstate->normal_matrix);
		glstate->normal_matrix_dirty = 0;
	}
	return glstate->normal_matrix;
}

static inline GLfloat* getPMat() {
	return glstate->projection_matrix->stack+glstate->projection_matrix->top*16;
}

static inline GLfloat* getMVPMat()
{
	if(glstate->mvp_matrix_dirty) {
		matrix_mul(getPMat(), getMVMat(), glstate->mvp_matrix);
		glstate->mvp_matrix_dirty = 0;
	}
	return glstate->mvp_matrix;
}


#endif // __COBALT_MATRIX_H_
