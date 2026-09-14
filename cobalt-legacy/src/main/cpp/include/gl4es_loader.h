#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool gl4es_load(const char* gles_path);
bool gl4es_init(int gles_level);
void gl4es_shutdown(void);
void* gl4es_get_proc_address(const char* name);
void gl4es_set_fb_size(int width, int height);
bool gl4es_is_loaded(void);
bool gl4es_is_initialized(void);

#ifdef __cplusplus
}
#endif
