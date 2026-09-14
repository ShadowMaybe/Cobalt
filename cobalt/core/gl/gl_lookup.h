#ifndef _GL_LOOKUP_H_
#define _GL_LOOKUP_H_

#include "attributes.h"

#ifdef DEBUG
#define MAP(func_name, func) \
    if (strcmp(name, func_name) == 0) {printf("%p (%s)\n", (void*)func, #func) ;return (void *)func;}
#else
#define MAP(func_name, func) \
    if (strcmp(name, func_name) == 0) return (void *)func;
#endif

#define EX(func_name) MAP(#func_name, func_name)

#define ARB(func_name) MAP(#func_name "ARB", func_name)

#define EXT(func_name) MAP(#func_name "EXT", func_name)

#define _EX(func_name) MAP(#func_name, cobalt_ ## func_name)

#define _ARB(func_name) MAP(#func_name "ARB", cobalt_ ## func_name)

#define _EXT(func_name) MAP(#func_name "EXT", cobalt_ ## func_name)

#ifndef STUB_FCT
#error STUB_FCT is not defined
#endif

#ifdef DEBUG
#define STUB(func_name)                       \
    if (strcmp(name, #func_name) == 0) {      \
        printf("=> STUB\n");                  \
        if(!g_cobalt.silentstub) LOGD("cobalt stub: %s\n", #func_name); \
        return (void *)STUB_FCT;              \
    }
#else
#define STUB(func_name)                       \
    if (strcmp(name, #func_name) == 0) {      \
        if(!g_cobalt.silentstub) LOGD("cobalt stub: %s\n", #func_name); \
        return (void *)STUB_FCT;              \
    }
#endif

NonAliasExportDecl(void*,cobalt_GetProcAddress,(const char *name));

#endif //_GL_LOOKUP_H_