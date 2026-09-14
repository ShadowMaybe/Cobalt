/**
 * Legacy route adapter native bridge for Cobalt.
 *
 * JNI bridge between Kotlin and the gl4es_extra_extra compatibility layer.
 * Handles library loading, initialization, shader translation, and diagnostics.
 *
 * Based on the actual gl4es_extra_extra API:
 *   - initialize_gl4es() / close_gl4es()
 *   - set_getprocaddress() / set_getmainfbsize()
 *   - gl4es_GetProcAddress()
 *   - Environment variables: LIBGL_ES, LIBGL_GL, LIBGL_FB, etc.
 */

#include <jni.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <string>
#include <cstring>
#include <dlfcn.h>

#include "gl4es_loader.h"
#include "legacy_bridge.h"

#define TAG "CobaltLegacy"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)

// ──────────────────────────────────────────────────────────────────────────────
// JNI Methods
// ──────────────────────────────────────────────────────────────────────────────

extern "C" {

/**
 * Initialize the legacy route adapter.
 * Loads gl4es or falls back to direct GLES, sets environment, initializes state.
 */
JNIEXPORT jboolean JNICALL
Java_me_shadow_cobalt_legacy_LegacyRouteAdapter_nativeInitialize(
    JNIEnv *env,
    jobject thiz,
    jstring glesLibraryPath,
    jstring eglLibraryPath,
    jobject config
) {
    const char *glesPath = env->GetStringUTFChars(glesLibraryPath, nullptr);
    const char *eglPath = env->GetStringUTFChars(eglLibraryPath, nullptr);

    LOGI("Initializing legacy adapter: GLES=%s, EGL=%s", glesPath, eglPath);

    // Load gl4es library
    bool loaded = gl4es_load(glesPath);

    if (loaded) {
        // Initialize gl4es with GLES 2.0 backend (default for legacy)
        bool ok = gl4es_init(2);
        if (!ok) {
            LOGE("Failed to initialize gl4es");
            env->ReleaseStringUTFChars(glesLibraryPath, glesPath);
            env->ReleaseStringUTFChars(eglLibraryPath, eglPath);
            return JNI_FALSE;
        }
        LOGI("Legacy adapter initialized with gl4es (GLES 2.0 backend)");
    } else {
        // Fallback: direct GLES 2.0 passthrough
        LOGW("gl4es not available, using direct GLES 2.0 passthrough");

        // Load GLES library directly
        if (strlen(glesPath) > 0) {
            void* handle = dlopen(glesPath, RTLD_LAZY);
            if (!handle) {
                LOGE("Failed to load GLES library: %s", glesPath);
            }
        }
        if (strlen(eglPath) > 0) {
            void* handle = dlopen(eglPath, RTLD_LAZY);
            if (!handle) {
                LOGE("Failed to load EGL library: %s", eglPath);
            }
        }
    }

    // Set additional environment variables for Minecraft compatibility
    setenv("LIBGL_USE_MC_COLOR", "1", 1);
    setenv("LIBGL_NORMALIZE", "1", 1);
    setenv("LIBGL_MIPMAP", "1", 1);
    setenv("LIBGL_NOERROR", "1", 1);
    setenv("LIBGL_SILENTSTUB", "1", 1);
    setenv("LIBGL_LOGSHADERERROR", "1", 1);

    env->ReleaseStringUTFChars(glesLibraryPath, glesPath);
    env->ReleaseStringUTFChars(eglLibraryPath, eglPath);

    return JNI_TRUE;
}

/**
 * Make the legacy context current.
 */
JNIEXPORT jboolean JNICALL
Java_me_shadow_cobalt_legacy_LegacyRouteAdapter_nativeMakeCurrent(
    JNIEnv *env,
    jobject thiz
) {
    // The EGL context is managed by Cobalt's SurfaceContextManager.
    // This method performs any gl4es-specific context activation if needed.
    LOGI("Legacy adapter: makeCurrent");
    return JNI_TRUE;
}

/**
 * Translate a desktop GLSL shader to GLES-compatible form.
 * Uses gl4es's shader conversion pipeline internally.
 */
JNIEXPORT jstring JNICALL
Java_me_shadow_cobalt_legacy_LegacyRouteAdapter_nativeTranslateShader(
    JNIEnv *env,
    jobject thiz,
    jint shaderType,
    jstring source
) {
    const char *src = env->GetStringUTFChars(source, nullptr);

    // gl4es handles shader conversion internally when shaders go through
    // the normal GL path. For direct translation, we apply basic transforms:
    // 1. Add ES precision qualifiers if missing
    // 2. Replace texture2D with texture for GLES 3.0+
    // 3. Handle built-in variable replacements

    std::string input(src);
    std::string output;

    // Check if already ESSL
    if (input.find("#version 100") != std::string::npos ||
        input.find("#version 300 es") != std::string::npos) {
        output = input; // Already ESSL
    } else {
        // Basic desktop GLSL -> ESSL transform
        output = input;

        // Add precision qualifier if missing
        if (output.find("precision") == std::string::npos) {
            if (shaderType == 0) { // Vertex
                output = "precision highp float;\n" + output;
            } else { // Fragment
                output = "precision mediump float;\n" + output;
            }
        }

        // Replace texture2D with texture for GLES 3.0+
        size_t pos = 0;
        while ((pos = output.find("texture2D(", pos)) != std::string::npos) {
            output.replace(pos, 10, "texture(");
            pos += 8;
        }

        // Replace texture2DLod with textureLod
        pos = 0;
        while ((pos = output.find("texture2DLod(", pos)) != std::string::npos) {
            output.replace(pos, 13, "textureLod(");
            pos += 10;
        }
    }

    jstring result = env->NewStringUTF(output.c_str());
    env->ReleaseStringUTFChars(source, src);
    return result;
}

/**
 * Perform a minimal draw to verify the context is functional.
 */
JNIEXPORT jboolean JNICALL
Java_me_shadow_cobalt_legacy_LegacyRouteAdapter_nativeMinimalDraw(
    JNIEnv *env,
    jobject thiz
) {
    LOGI("Legacy adapter: minimal draw health check");

    // Clear to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Check for errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("Legacy adapter: GL error during minimal draw: 0x%x", err);
        return JNI_FALSE;
    }

    LOGI("Legacy adapter: minimal draw passed");
    return JNI_TRUE;
}

/**
 * Set framebuffer size (called when surface is created/resized).
 */
JNIEXPORT void JNICALL
Java_me_shadow_cobalt_legacy_LegacyRouteAdapter_nativeSetFramebufferSize(
    JNIEnv *env,
    jobject thiz,
    jint width,
    jint height
) {
    gl4es_set_fb_size(width, height);
    LOGI("Legacy adapter: framebuffer size set to %dx%d", width, height);
}

/**
 * Destroy the legacy adapter and release all resources.
 */
JNIEXPORT void JNICALL
Java_me_shadow_cobalt_legacy_LegacyRouteAdapter_nativeDestroy(
    JNIEnv *env,
    jobject thiz
) {
    LOGI("Legacy adapter: destroying");
    gl4es_shutdown();
}

/**
 * Collect diagnostic data from the legacy adapter.
 */
JNIEXPORT jstring JNICALL
Java_me_shadow_cobalt_legacy_LegacyRouteAdapter_nativeCollectDiagnostics(
    JNIEnv *env,
    jobject thiz
) {
    // Build a JSON diagnostics object
    std::string diag = "{";
    diag += "\"gl4es_loaded\":" + std::string(gl4es_is_loaded() ? "true" : "false");
    diag += ",\"gl4es_initialized\":" + std::string(gl4es_is_initialized() ? "true" : "false");

    // Query current GL state
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);
    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);

    if (vendor) diag += ",\"vendor\":\"" + std::string(vendor) + "\"";
    if (renderer) diag += ",\"renderer\":\"" + std::string(renderer) + "\"";
    if (version) diag += ",\"gl_version\":\"" + std::string(version) + "\"";

    // Count extensions
    int extCount = 0;
    if (extensions) {
        const char* p = extensions;
        while (*p) { extCount++; while (*p && *p != ' ') p++; if (*p) p++; }
    }
    diag += ",\"extension_count\":" + std::to_string(extCount);

    // GL state queries
    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    diag += ",\"max_texture_size\":" + std::to_string(maxTextureSize);

    GLint maxVertexAttrs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttrs);
    diag += ",\"max_vertex_attribs\":" + std::to_string(maxVertexAttrs);

    diag += "}";
    return env->NewStringUTF(diag.c_str());
}

} // extern "C"
