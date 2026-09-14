/*
 * Cobalt Renderer — JNI Bridge
 * Connects Android Kotlin/Java to the unified native rendering engine.
 */

#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <dlfcn.h>
#include <cstring>

#define COBALT_TAG "Cobalt"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  COBALT_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  COBALT_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, COBALT_TAG, __VA_ARGS__)

// ==================== Forward declarations ====================
// These are defined in the merged engine (cobalt.c and modern/main.cpp)
// We use dlsym-like loading to find them at runtime.

// Legacy path (from cobalt.c)
typedef void (*legacy_init_fn)(void);
typedef void (*legacy_set_proc_address_fn)(void *(*)(const char *));
typedef void (*legacy_set_fb_size_fn)(void (*)(int *, int *));
typedef void (*legacy_shutdown_fn)(void);

// Modern path (from modern/main.cpp)
typedef void (*modern_init_fn)(void);

// ==================== Renderer state ====================
static struct {
    bool initialized;
    bool use_legacy;    // true = <=1.17, false = >1.17

    // Function pointers loaded from the engine
    legacy_init_fn        legacy_init;
    legacy_set_proc_address_fn legacy_set_proc;
    legacy_set_fb_size_fn legacy_set_fb;
    legacy_shutdown_fn    legacy_shutdown;
    modern_init_fn        modern_init;

    // EGL state
    ANativeWindow *window;
    int width;
    int height;
} cobalt_state = {};

// ==================== Core JNI exports ====================

extern "C" {

JNIEXPORT jint JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeInit(
        JNIEnv *env, jobject thiz,
        jboolean useLegacy,
        jint majorVersion, jint minorVersion) {

    LOGI("Cobalt nativeInit: legacy=%d, version=%d.%d",
         useLegacy, majorVersion, minorVersion);

    cobalt_state.use_legacy = useLegacy;
    cobalt_state.initialized = true;

    if (useLegacy) {
        // Legacy path: use the gl4es-derived engine
        // The actual initialization happens via the constructor in cobalt.c
        LOGI("Cobalt: Legacy route (gl4es-derived engine)");
    } else {
        // Modern path: use the MobileGlues-derived engine
        // The actual initialization happens via static constructor in modern/init.cpp
        LOGI("Cobalt: Modern route (MobileGlues-derived engine)");
    }

    return 0; // success
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeSetSurface(
        JNIEnv *env, jobject thiz,
        jobject surface) {

    if (surface == nullptr) {
        cobalt_state.window = nullptr;
        LOGI("Cobalt: Surface cleared");
        return;
    }

    cobalt_state.window = ANativeWindow_fromSurface(env, surface);
    if (cobalt_state.window) {
        cobalt_state.width = ANativeWindow_getWidth(cobalt_state.window);
        cobalt_state.height = ANativeWindow_getHeight(cobalt_state.window);
        LOGI("Cobalt: Surface set %dx%d", cobalt_state.width, cobalt_state.height);
    } else {
        LOGE("Cobalt: Failed to get native window");
    }
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeSetFramebufferSize(
        JNIEnv *env, jobject thiz,
        jint width, jint height) {

    cobalt_state.width = width;
    cobalt_state.height = height;
    LOGI("Cobalt: Framebuffer size set %dx%d", width, height);
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeSetEnvironmentVariable(
        JNIEnv *env, jobject thiz,
        jstring key, jstring value) {

    const char *k = env->GetStringUTFChars(key, nullptr);
    const char *v = env->GetStringUTFChars(value, nullptr);

    setenv(k, v, 1);
    LOGI("Cobalt: Env var set %s=%s", k, v);

    env->ReleaseStringUTFChars(key, k);
    env->ReleaseStringUTFChars(value, v);
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeSetConfigPath(
        JNIEnv *env, jobject thiz,
        jstring path) {

    const char *p = env->GetStringUTFChars(path, nullptr);
    setenv("COBALT_CONFIG_PATH", p, 1);
    LOGI("Cobalt: Config path set to %s", p);
    env->ReleaseStringUTFChars(path, p);
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeShutdown(
        JNIEnv *env, jobject thiz) {

    LOGI("Cobalt: Shutting down");

    if (cobalt_state.window) {
        ANativeWindow_release(cobalt_state.window);
        cobalt_state.window = nullptr;
    }

    cobalt_state.initialized = false;
}

JNIEXPORT jstring JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeGetVersion(
        JNIEnv *env, jobject thiz) {

    return env->NewStringUTF("1.0.0");
}

JNIEXPORT jstring JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeGetRendererName(
        JNIEnv *env, jobject thiz) {

    return env->NewStringUTF("Cobalt");
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGI("Cobalt: JNI_OnLoad called");
    return JNI_VERSION_1_6;
}

} // extern "C"
