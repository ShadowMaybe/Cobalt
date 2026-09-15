/*
 * Cobalt Renderer — JNI Bridge
 * Routes between legacy and modern native engines.
 */

#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>

#define TAG "Cobalt"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ==================== Renderer state ====================
static struct {
    bool initialized;
    bool use_legacy;
    void *lib_handle;   // dlopen handle for the loaded engine
} state = {};

// ==================== Helper: load engine library ====================
static bool load_engine(bool use_legacy) {
    const char *lib_name = use_legacy ? "libcobalt_legacy.so" : "libcobalt_modern.so";
    state.lib_handle = dlopen(lib_name, RTLD_NOW | RTLD_GLOBAL);
    if (!state.lib_handle) {
        LOGE("Failed to load %s: %s", lib_name, dlerror());
        return false;
    }
    LOGI("Loaded %s", lib_name);
    return true;
}

// ==================== JNI exports ====================

extern "C" {

JNIEXPORT jint JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeInit(
        JNIEnv *env, jobject thiz,
        jboolean useLegacy,
        jint majorVersion, jint minorVersion) {

    LOGI("nativeInit: legacy=%d, version=%d.%d", useLegacy, majorVersion, minorVersion);

    state.use_legacy = useLegacy;

    if (!load_engine(useLegacy)) {
        return -1;
    }

    state.initialized = true;

    // The engine's own static constructors run on dlopen — no extra init needed here.
    LOGI("Engine loaded successfully");
    return 0;
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeSetEnvironmentVariable(
        JNIEnv *env, jobject thiz,
        jstring key, jstring value) {

    const char *k = env->GetStringUTFChars(key, nullptr);
    const char *v = env->GetStringUTFChars(value, nullptr);
    setenv(k, v, 1);
    env->ReleaseStringUTFChars(key, k);
    env->ReleaseStringUTFChars(value, v);
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeSetConfigPath(
        JNIEnv *env, jobject thiz,
        jstring path) {
    const char *p = env->GetStringUTFChars(path, nullptr);
    setenv("COBALT_CONFIG_PATH", p, 1);
    env->ReleaseStringUTFChars(path, p);
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeSetFramebufferSize(
        JNIEnv *env, jobject thiz,
        jint width, jint height) {
    LOGI("Framebuffer size: %dx%d", width, height);
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeSetSurface(
        JNIEnv *env, jobject thiz, jobject surface) {
    // Surface management delegated to the engine
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_CobaltRenderer_nativeShutdown(
        JNIEnv *env, jobject thiz) {

    LOGI("Shutting down");

    if (state.lib_handle) {
        dlclose(state.lib_handle);
        state.lib_handle = nullptr;
    }

    state.initialized = false;
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
    LOGI("JNI_OnLoad");
    return JNI_VERSION_1_6;
}

} // extern "C"
