#ifndef __COBALT_UNIFORM_H_
#define __COBALT_UNIFORM_H_

#include "gles.h"

int uniformsize(GLenum type);
int is_uniform_int(GLenum type);
int is_uniform_float(GLenum type);
int is_uniform_matrix(GLenum type);
int n_uniform(GLenum type);

void COBALT_API cobalt_glGetUniformfv(GLuint program, GLint location, GLfloat *params);
void COBALT_API cobalt_glGetUniformiv(GLuint program, GLint location, GLint *params);

void COBALT_API cobalt_glUniform1f(GLint location, GLfloat v0);
void COBALT_API cobalt_glUniform2f(GLint location, GLfloat v0, GLfloat v1);
void COBALT_API cobalt_glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void COBALT_API cobalt_glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void COBALT_API cobalt_glUniform1i(GLint location, GLint v0);
void COBALT_API cobalt_glUniform2i(GLint location, GLint v0, GLint v1); 
void COBALT_API cobalt_glUniform3i(GLint location, GLint v0, GLint v1, GLint v2); 
void COBALT_API cobalt_glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);

void COBALT_API cobalt_glUniform1fv(GLint location, GLsizei count, const GLfloat *value); 
void COBALT_API cobalt_glUniform2fv(GLint location, GLsizei count, const GLfloat *value); 
void COBALT_API cobalt_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
void COBALT_API cobalt_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
void COBALT_API cobalt_glUniform1iv(GLint location, GLsizei count, const GLint *value);
void COBALT_API cobalt_glUniform2iv(GLint location, GLsizei count, const GLint *value);
void COBALT_API cobalt_glUniform3iv(GLint location, GLsizei count, const GLint *value);
void COBALT_API cobalt_glUniform4iv(GLint location, GLsizei count, const GLint *value);

void COBALT_API cobalt_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void COBALT_API cobalt_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void COBALT_API cobalt_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

// ========== GL_ARB_shader_objects ==============
GLvoid COBALT_API glUniform1fARB(GLint location, GLfloat v0);
GLvoid COBALT_API glUniform2fARB(GLint location, GLfloat v0, GLfloat v1);
GLvoid COBALT_API glUniform3fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GLvoid COBALT_API glUniform4fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLvoid COBALT_API glUniform1iARB(GLint location, GLint v0);
GLvoid COBALT_API glUniform2iARB(GLint location, GLint v0, GLint v1);
GLvoid COBALT_API glUniform3iARB(GLint location, GLint v0, GLint v1, GLint v2);
GLvoid COBALT_API glUniform4iARB(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GLvoid COBALT_API glUniform1fvARB(GLint location, GLsizei count, const GLfloat *value);
GLvoid COBALT_API glUniform2fvARB(GLint location, GLsizei count, const GLfloat *value);
GLvoid COBALT_API glUniform3fvARB(GLint location, GLsizei count, const GLfloat *value);
GLvoid COBALT_API glUniform4fvARB(GLint location, GLsizei count, const GLfloat *value);
GLvoid COBALT_API glUniform1ivARB(GLint location, GLsizei count, const GLint *value);
GLvoid COBALT_API glUniform2ivARB(GLint location, GLsizei count, const GLint *value);
GLvoid COBALT_API glUniform3ivARB(GLint location, GLsizei count, const GLint *value);
GLvoid COBALT_API glUniform4ivARB(GLint location, GLsizei count, const GLint *value);
GLvoid COBALT_API glUniformMatrix2fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid COBALT_API glUniformMatrix3fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLvoid COBALT_API glUniformMatrix4fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
// ===========
void COBALT_API cobalt_glProgramUniform1f(GLuint program, GLint location, GLfloat v0);
void COBALT_API cobalt_glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1);
void COBALT_API cobalt_glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void COBALT_API cobalt_glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void COBALT_API cobalt_glProgramUniform1i(GLuint program, GLint location, GLint v0);
void COBALT_API cobalt_glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1); 
void COBALT_API cobalt_glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2); 
void COBALT_API cobalt_glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);

void COBALT_API cobalt_glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value); 
void COBALT_API cobalt_glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value); 
void COBALT_API cobalt_glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
void COBALT_API cobalt_glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
void COBALT_API cobalt_glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value);
void COBALT_API cobalt_glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value);
void COBALT_API cobalt_glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value);
void COBALT_API cobalt_glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value);

void COBALT_API cobalt_glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void COBALT_API cobalt_glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void COBALT_API cobalt_glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);


#endif // __COBALT_UNIFORM_H_
