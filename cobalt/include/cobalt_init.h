
#ifndef _cobaltINCLUDE_INIT_H_
#define _cobaltINCLUDE_INIT_H_

#ifndef COBALT_API
# if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(__SCITECH_SNAP__)
#  define COBALT_API __stdcall
# else
#  define COBALT_API
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// set driver GetProcAddress implementation. required for hardext detection with NOEGL or when loader is disabled
void set_getprocaddress(void *(COBALT_API *new_proc_address)(const char *));
// reguired with NOEGL
void set_getmainfbsize(void (COBALT_API *new_getMainFBSize)(int* width, int* height));
// do this before any GL calls if init constructors are disabled.
void cobalt_init(void);
// do this to uninitialize cobalt if init constructors are disabled.
void cobalt_shutdown(void);
// wrapped GetProcAddress
void* COBALT_API cobalt_GetProcAddress(const char *name);

#ifdef __cplusplus
}
#endif

#endif
