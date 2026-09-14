#ifndef __COBALT_DEBUG_H_
#define __COBALT_DEBUG_H_

#include "gles.h"

const char* PrintHex(GLenum what);

const char* PrintEnum(GLenum what);

const char* PrintEGLError(int onlyerror);

void CheckGLError(int fwd);

#endif // __COBALT_DEBUG_H_
