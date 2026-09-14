#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MobileGlues dynamic loader.
 *
 * Loads libmobileglues.so at runtime. MobileGlues auto-initializes
 * via a static constructor when loaded, reading config from
 * /sdcard/MG/config.json and probing GLES capabilities.
 *
 * The host must set these environment variables BEFORE loading:
 *   - MG_DIR_PATH: base directory (default /sdcard/MG)
 *   - MG_PLUGIN_STATUS: "1" to signal a known launcher
 *   - MG_ANGLE_DIR: path to ANGLE libraries (optional)
 */

bool mg_load(const char* mg_lib_path);
void mg_unload(void);
bool mg_is_loaded(void);

// Environment setup (must be called before mg_load)
void mg_set_env(const char* key, const char* value);

#ifdef __cplusplus
}
#endif
