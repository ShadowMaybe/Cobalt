#include "host.h"

//#define DEBUG
#ifdef DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif

    #define INIT_EGL(name) \
        host_functions.name = (name##_PTR) proc_address(egl, "eglGetProcAddress"); \
        DBG(WARN_NULL(host_functions.name);); \
    
    #define INIT_GLES_FPE(name) \
        host_functions.fpe_##name = (name##_PTR) ((g_cobalt.es == 1) ? proc_address(gles, #name) : (void*) fpe_##name); \
        DBG(WARN_NULL(host_functions.fpe_##name);); \
    
    #define INIT_GLES(name) \
        host_functions.name = (name##_PTR) proc_address(gles, #name); \
        DBG(WARN_NULL(host_functions.name);); \
    
    #define INIT_GLES_OR_OES(name) \
        if(g_cobalt.es > 1) { host_functions.name = (name##_PTR) proc_address(gles, #name); } else { host_functions.name = (name##_PTR) host_functions.eglGetProcAddress(#name"OES"); } \
        DBG(WARN_NULL(host_functions.name);); \
    
    #define INIT_GLES_EXT(name) \
        host_functions.name = (name##_PTR) host_functions.eglGetProcAddress(#name"EXT"); \
        DBG(WARN_NULL(host_functions.name);); \
    
    #define INIT_GLES_IF_EXT(name, hardext_name) \
        if(hardext_name) { host_functions.name =  (name##_PTR) host_functions.eglGetProcAddress(#name"EXT"); } else { host_functions.name =  (name##_PTR) host_functions.eglGetProcAddress(#name); } \
        DBG(WARN_NULL(host_functions.name);); \
    
    #define INIT_GLES_OES(name) \
        host_functions.name = (name##_PTR) host_functions.eglGetProcAddress(#name"OES"); \
        DBG(WARN_NULL(host_functions.name);); \
    
    gl_functions host_functions = {};
void init_gl_functions() {
    ALL_FUNCTIONS(); 
};
#undef INIT_GLES_FPE
#undef INIT_GLES
#undef INIT_GLES_OR_OES
#undef INIT_GLES_EXT
#undef INIT_GLES_IF_EXT
#undef INIT_GLES_OES
#undef INIT_EGL
