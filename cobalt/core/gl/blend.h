#ifndef __COBALT_BLEND_H_
#define __COBALT_BLEND_H_

#include "gles.h"

void COBALT_API cobalt_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void COBALT_API cobalt_glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
void COBALT_API cobalt_glBlendEquationSeparate(GLenum modeRGB, GLenum modeA);
void COBALT_API cobalt_glBlendFunc(GLenum sfactor, GLenum dfactor);
void COBALT_API cobalt_glBlendEquation(GLenum mode);

#endif //__COBALT_BLEND_H_
