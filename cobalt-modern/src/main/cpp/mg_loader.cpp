/**
 * MobileGlues dynamic loader.
 *
 * MobileGlues self-initializes via a static constructor when loaded.
 * The host's responsibility is:
 * 1. Set environment variables (MG_DIR_PATH, MG_PLUGIN_STATUS, etc.)
 * 2. dlopen libmobileglues.so which triggers proc_init()
 * 3. The library reads /sdcard/MG/config.json and probes GLES
 */

#include <jni.h>
#include <dlfcn.h>
#include <cstdlib>
#include <android/log.h>
#include <string>

#define TAG "CobaltModernLoader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static void* g_mg_handle = nullptr;
static bool g_loaded = false;

// MobileGlues exported function (if available)
typedef int (*mg_angle_in_use_t)(void);
static mg_angle_in_use_t g_mg_angle_in_use = nullptr;

// ──────────────────────────────────────────────────────────────────────────────
// Public API — must match the extern "C" declarations in mg_loader.h
// ──────────────────────────────────────────────────────────────────────────────

extern "C" {

void mg_set_env(const char* key, const char* value) {
    setenv(key, value, 1);
    LOGI("Set env: %s=%s", key, value);
}

/**
 * Load MobileGlues library.
 *
 * MobileGlues is the shader translation and GL state management layer.
 *   - Self-initializes via a static constructor (proc_init)
 *   - Invoked via eglGetProcAddress or dlsym.
 *   - Reads GLSL cache from /sdcard/MG/glsl_cache.tmp
 *   - Supports ANGLE as an alternative backend
 */

bool mg_load(const char* mg_lib_path) {
    if (g_mg_handle) {
        LOGI("MobileGlues already loaded");
        return true;
    }

    const char* paths[] = {
        mg_lib_path,
        "libmobileglues.so",
        nullptr
    };

    for (int i = 0; paths[i] != nullptr; i++) {
        if (paths[i] == nullptr || strlen(paths[i]) == 0) continue;

        g_mg_handle = dlopen(paths[i], RTLD_LAZY | RTLD_GLOBAL);
        if (g_mg_handle) {
            LOGI("Loaded MobileGlues from: %s", paths[i]);

            // Resolve optional exported functions
            g_mg_angle_in_use = (mg_angle_in_use_t)dlsym(g_mg_handle, "mg_angle_in_use");

            g_loaded = true;
            return true;
        }
    }

    LOGE("Failed to load MobileGlues from any path");
    return false;
}

void mg_unload(void) {
    if (g_mg_handle) {
        dlclose(g_mg_handle);
        g_mg_handle = nullptr;
        g_loaded = false;
        g_mg_angle_in_use = nullptr;
        LOGI("MobileGlues unloaded");
    }
}

bool mg_is_loaded(void) {
    return g_loaded;
}

bool mg_angle_in_use(void) {
    if (g_mg_angle_in_use) {
        return g_mg_angle_in_use() == 1;
    }
    return false;
}

} // extern "C"
