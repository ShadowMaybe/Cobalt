#pragma once

#include <jni.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Lifecycle
bool legacy_init(const char* gles_path, const char* egl_path, int gles_level);
void legacy_shutdown(void);
bool legacy_make_current(void);
bool legacy_minimal_draw(void);
void legacy_set_framebuffer_size_callback(int width, int height);

// Shader
const char* legacy_translate_shader(int shader_type, const char* source);

// Diagnostics
const char* legacy_get_info_json(void);
const char* legacy_get_extensions(void);
int legacy_get_gles_version(void);

#ifdef __cplusplus
}
#endif
