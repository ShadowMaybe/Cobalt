#ifndef __COBALT_DEPTH_H_
#define __COBALT_DEPTH_H_

#include "gles.h"

void COBALT_API cobalt_glDepthFunc(GLenum func);
void COBALT_API cobalt_glDepthMask(GLboolean flag);
void COBALT_API cobalt_glDepthRangef(GLclampf nearVal, GLclampf farVal);
void COBALT_API cobalt_glClearDepthf(GLclampf depth);

#endif // __COBALT_DEPTH_H_
