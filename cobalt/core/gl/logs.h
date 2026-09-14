#ifndef __COBALT_LOGS_H_
#define __COBALT_LOGS_H_
//----------------------------------------------------------------------------
#include <stdio.h>
#include "init.h"
#include "attributes.h"
//----------------------------------------------------------------------------
void LogPrintf_NoPrefix(const char *fmt,...);
void LogFPrintf(FILE *fp,const char *fmt,...);
EXPORT void LogPrintf(const char *fmt,...);
//----------------------------------------------------------------------------
#ifdef COBALT_SILENCE_MESSAGES
	#define COBALT_LOGD(...)
	#define COBALT_LOGD_NOPREFIX(...)
	#define COBALT_LOGE(...)
#else
	#define COBALT_LOGD(...) if(!g_cobalt.nobanner) LogPrintf(__VA_ARGS__)
	#define COBALT_LOGD_NOPREFIX(...) if(!g_cobalt.nobanner) LogPrintf_NoPrefix(__VA_ARGS__)
	#define COBALT_LOGE(...) if(!g_cobalt.nobanner) LogFPrintf(stderr,__VA_ARGS__)
#endif
//----------------------------------------------------------------------------
#define LOGD(...) LogPrintf(__VA_ARGS__)
#define LOGE(...) LogFPrintf(stderr,__VA_ARGS__)
//----------------------------------------------------------------------------
#endif // __COBALT_LOGS_H_
