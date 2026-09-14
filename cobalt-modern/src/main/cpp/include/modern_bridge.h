#pragma once

#include <jni.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Lifecycle
bool modern_init(const char* mg_dir_path, const char* angle_dir, bool enable_anonymous);
void modern_shutdown(void);
bool modern_make_current(void);
bool modern_minimal_draw(void);

// Diagnostics
const char* modern_get_gl_info(void);
int modern_get_gles_version(void);
bool modern_angle_in_use(void);

// Shader
const char* modern_translate_shader(const char* source, int version, const char* profile);

#ifdef __cplusplus
}
#endif
