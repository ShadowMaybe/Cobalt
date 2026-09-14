/**
 * gl4es_extra_extra dynamic loader.
 *
 * Loads libgl4es_114.so at runtime and resolves the 5 required entry points:
 *   - initialize_gl4es()
 *   - close_gl4es()
 *   - set_getprocaddress()
 *   - set_getmainfbsize()
 *   - gl4es_GetProcAddress()
 *
 * If the library is not found, falls back to direct GLES 2.0 passthrough.
 */

#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <string>

#define TAG "CobaltLegacyLoader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)

// gl4es function pointers
typedef void (*initialize_gl4es_t)(void);
typedef void (*close_gl4es_t)(void);
typedef void (*set_getprocaddress_t)(void *(*)(const char *));
typedef void (*set_getmainfbsize_t)(void (*)(int *, int *));
typedef void* (*gl4es_GetProcAddress_t)(const char *);

static void* g_gl4es_handle = nullptr;
static initialize_gl4es_t g_initialize_gl4es = nullptr;
static close_gl4es_t g_close_gl4es = nullptr;
static set_getprocaddress_t g_set_getprocaddress = nullptr;
static set_getmainfbsize_t g_set_getmainfbsize = nullptr;
static gl4es_GetProcAddress_t g_gl4es_GetProcAddress = nullptr;

static bool g_initialized = false;
static int g_gles_level = 2;
static int g_fb_width = 320;
static int g_fb_height = 240;

// Framebuffer size callback for gl4es
static void default_fb_size_callback(int* w, int* h) {
    *w = g_fb_width;
    *h = g_fb_height;
}

/**
 * Load the gl4es library and resolve all required symbols.
 * Returns true if the library was loaded successfully.
 */
bool gl4es_load(const char* gles_path) {
    if (g_gl4es_handle) {
        LOGI("gl4es already loaded");
        return true;
    }

    // Try to load gl4es from the provided path or system search
    const char* lib_names[] = {
        gles_path,
        "libgl4es_114.so",
        "libgl4es.so",
        nullptr
    };

    for (int i = 0; lib_names[i] != nullptr; i++) {
        if (lib_names[i] == nullptr || strlen(lib_names[i]) == 0) continue;

        g_gl4es_handle = dlopen(lib_names[i], RTLD_LAZY | RTLD_LOCAL);
        if (g_gl4es_handle) {
            LOGI("Loaded gl4es from: %s", lib_names[i]);
            break;
        }
    }

    if (!g_gl4es_handle) {
        LOGW("gl4es library not found, will use direct GLES passthrough");
        return false;
    }

    // Resolve symbols
    g_initialize_gl4es = (initialize_gl4es_t)dlsym(g_gl4es_handle, "initialize_gl4es");
    g_close_gl4es = (close_gl4es_t)dlsym(g_gl4es_handle, "close_gl4es");
    g_set_getprocaddress = (set_getprocaddress_t)dlsym(g_gl4es_handle, "set_getprocaddress");
    g_set_getmainfbsize = (set_getmainfbsize_t)dlsym(g_gl4es_handle, "set_getmainfbsize");
    g_gl4es_GetProcAddress = (gl4es_GetProcAddress_t)dlsym(g_gl4es_handle, "gl4es_GetProcAddress");

    if (!g_initialize_gl4es || !g_close_gl4es) {
        LOGE("Failed to resolve required gl4es symbols");
        dlclose(g_gl4es_handle);
        g_gl4es_handle = nullptr;
        return false;
    }

    LOGI("gl4es symbols resolved successfully");
    return true;
}

/**
 * Initialize the gl4es library with the specified configuration.
 * Must be called after loading and before any GL calls.
 */
bool gl4es_init(int gles_level) {
    if (g_initialized) {
        LOGI("gl4es already initialized");
        return true;
    }

    g_gles_level = gles_level;

    // Set environment variables before initialization
    // These control gl4es behavior
    setenv("LIBGL_ES", std::to_string(gles_level).c_str(), 1);
    setenv("LIBGL_GL", gles_level >= 2 ? "31" : "21", 1);
    setenv("LIBGL_FB", "3", 1);  // PBuffer mode for headless
    setenv("LIBGL_NOBANNER", "1", 1);
    setenv("LIBGL_NOERROR", "1", 1);
    setenv("LIBGL_NORMALIZE", "1", 1);
    setenv("LIBGL_MIPMAP", "1", 1);
    setenv("LIBGL_SHRINK", "0", 1);
    setenv("LIBGL_FORCENPOT", "0", 1);
    setenv("LIBGL_SILENTSTUB", "1", 1);
    setenv("LIBGL_LOGSHADERERROR", "1", 1);
    setenv("LIBGL_COMMENTS", "0", 1);

    // Set framebuffer size callback
    if (g_set_getmainfbsize) {
        g_set_getmainfbsize(default_fb_size_callback);
    }

    // Initialize gl4es
    if (g_initialize_gl4es) {
        g_initialize_gl4es();
        LOGI("gl4es initialized with GLES level: %d", gles_level);
    }

    g_initialized = true;
    return true;
}

/**
 * Shutdown gl4es and release resources.
 */
void gl4es_shutdown(void) {
    if (!g_initialized) return;

    if (g_close_gl4es) {
        g_close_gl4es();
    }

    if (g_gl4es_handle) {
        dlclose(g_gl4es_handle);
        g_gl4es_handle = nullptr;
    }

    g_initialized = false;
    g_initialize_gl4es = nullptr;
    g_close_gl4es = nullptr;
    g_set_getprocaddress = nullptr;
    g_set_getmainfbsize = nullptr;
    g_gl4es_GetProcAddress = nullptr;

    LOGI("gl4es shutdown complete");
}

/**
 * Get the underlying GLES function pointer for a given name.
 */
void* gl4es_get_proc_address(const char* name) {
    if (g_gl4es_GetProcAddress) {
        return g_gl4es_GetProcAddress(name);
    }
    // Fallback to EGL's getProcAddress
    return (void*)eglGetProcAddress(name);
}

/**
 * Set the framebuffer size reported to gl4es.
 */
void gl4es_set_fb_size(int width, int height) {
    g_fb_width = width;
    g_fb_height = height;
}

bool gl4es_is_loaded(void) {
    return g_gl4es_handle != nullptr;
}

bool gl4es_is_initialized(void) {
    return g_initialized;
}
