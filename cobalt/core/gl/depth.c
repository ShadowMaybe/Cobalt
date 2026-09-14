#include "host.h"
#include "depth.h"

#include "cobalt.h"
#include "glstate.h"
#include "loader.h"

void COBALT_API cobalt_glDepthFunc(GLenum func) {
    if(glstate->list.compiling) {
        PUSH_IF_COMPILING(glDepthFunc);
    }
    noerrorShim();
    if (glstate->depth.func == func)
        return;
    FLUSH_BEGINEND;
    glstate->depth.func = func;
    
    errorGL();
    host_functions.glDepthFunc(func);
}

void COBALT_API cobalt_glDepthMask(GLboolean flag) {
    if(glstate->list.compiling) {
        PUSH_IF_COMPILING(glDepthMask);
    }
    noerrorShim();
    if (glstate->depth.mask == flag)
        return;
    FLUSH_BEGINEND;
    glstate->depth.mask = flag;
    
    errorGL();
    host_functions.glDepthMask(flag);
}

GLfloat clamp(GLfloat a) {
    return (a<0.f)?0.f:((a>1.f)?1.f:a);
}

void COBALT_API cobalt_glDepthRangef(GLclampf Near, GLclampf Far) {
    Near = clamp(Near);
    Far = clamp(Far);
    if(glstate->list.compiling) {
        PUSH_IF_COMPILING(glDepthRangef);
    }
    noerrorShim();
    if ((glstate->depth.Near == Near) && (glstate->depth.Far == Far))
        return;
    FLUSH_BEGINEND;
    glstate->depth.Near = Near;
    glstate->depth.Far = Far;
    
    errorGL();
    host_functions.glDepthRangef(Near, Far);
}

void COBALT_API cobalt_glClearDepthf(GLclampf depth) {
    depth = clamp(depth);
    if(glstate->list.compiling) {
        PUSH_IF_COMPILING(glClearDepthf);
    }
    noerrorShim();
    glstate->depth.clear = depth;
    
    errorGL();
    host_functions.glClearDepthf(depth);
}

AliasExport(void,glDepthFunc,,(GLenum func));
AliasExport(void,glDepthMask,,(GLboolean flag));
AliasExport(void,glDepthRangef,,(GLclampf nearVal, GLclampf farVal));
AliasExport(void,glClearDepthf,,(GLclampf depth));

