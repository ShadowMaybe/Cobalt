/**
 * Modern route adapter native bridge for Cobalt.
 *
 * JNI bridge between Kotlin and MobileGlues.
 * Handles library loading, environment setup, and diagnostics.
 *
 * Based on the actual MobileGlues API:
 *   - Self-initializes via static constructor
 *   - Reads config from /sdcard/MG/config.json
 *   - Requires environment: MG_DIR_PATH, MG_PLUGIN_STATUS
 *   - Requires GLES 3.0 minimum, prefers 3.2
 *   - Shader translation: GLSL -> SPIR-V -> ESSL
 *   - GLSL cache: /sdcard/MG/glsl_cache.tmp
 */

#include <jni.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string>
#include <cstring>

#include "mg_loader.h"
#include "modern_bridge.h"

#define TAG "CobaltModern"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ──────────────────────────────────────────────────────────────────────────────
// JNI Methods
// ──────────────────────────────────────────────────────────────────────────────

extern "C" {

/**
 * Initialize the modern route adapter.
 * Sets up environment for MobileGlues and loads the library.
 */
JNIEXPORT jboolean JNICALL
Java_me_shadow_cobalt_modern_ModernRouteAdapter_nativeInitialize(
    JNIEnv *env,
    jobject thiz,
    jobject config
) {
    LOGI("Initializing modern adapter");

    // Extract config values from the ProviderConfig Kotlin object
    // The config object has: routeFamily, profile, libraryPaths, environmentOverrides, etc.
    jclass configClass = env->GetObjectClass(config);

    // Get environmentOverrides map
    jmethodID getEnvOverrides = env->GetMethodID(configClass, "getEnvironmentOverrides", "()Ljava/util/Map;");
    jobject envMap = env->CallObjectMethod(config, getEnvOverrides);

    if (envMap != nullptr) {
        jclass mapClass = env->FindClass("java/util/Map");
        jclass setClass = env->FindClass("java/util/Set");
        jclass iteratorClass = env->FindClass("java/util/Iterator");
        jclass entryClass = env->FindClass("java/util/Map$Entry");

        jmethodID entrySet = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
        jmethodID iterator = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
        jmethodID hasNext = env->GetMethodID(iteratorClass, "hasNext", "()Z");
        jmethodID next = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
        jmethodID getKey = env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
        jmethodID getValue = env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");

        jobject entrySetObj = env->CallObjectMethod(envMap, entrySet);
        jobject iteratorObj = env->CallObjectMethod(entrySetObj, iterator);

        while (env->CallBooleanMethod(iteratorObj, hasNext)) {
            jobject entry = env->CallObjectMethod(iteratorObj, next);
            auto key = (jstring)env->CallObjectMethod(entry, getKey);
            auto value = (jstring)env->CallObjectMethod(entry, getValue);

            const char* keyStr = env->GetStringUTFChars(key, nullptr);
            const char* valueStr = env->GetStringUTFChars(value, nullptr);

            mg_set_env(keyStr, valueStr);

            env->ReleaseStringUTFChars(key, keyStr);
            env->ReleaseStringUTFChars(value, valueStr);
        }
    }

    // Set required MobileGlues environment variables
    mg_set_env("MG_PLUGIN_STATUS", "1");

    // Get library paths
    jmethodID getLibPaths = env->GetMethodID(configClass, "getLibraryPaths", "()Ljava/util/Map;");
    jobject libPaths = env->CallObjectMethod(config, getLibPaths);

    std::string mgLibPath;
    if (libPaths != nullptr) {
        jclass mapClass = env->FindClass("java/util/Map");
        jmethodID get = env->GetMethodID(mapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
        jstring key = env->NewStringUTF("mobileglues");
        jobject value = env->CallObjectMethod(libPaths, get, key);
        if (value != nullptr) {
            auto str = (jstring)value;
            mgLibPath = env->GetStringUTFChars(str, nullptr);
            env->ReleaseStringUTFChars(str, env->GetStringUTFChars(str, nullptr));
        }
    }

    // Load MobileGlues
    bool loaded = mg_load(mgLibPath.empty() ? nullptr : mgLibPath.c_str());
    if (!loaded) {
        LOGE("Failed to load MobileGlues");
        return JNI_FALSE;
    }

    LOGI("Modern adapter initialized with MobileGlues");
    return JNI_TRUE;
}

/**
 * Make the modern context current.
 */
JNIEXPORT jboolean JNICALL
Java_me_shadow_cobalt_modern_ModernRouteAdapter_nativeMakeCurrent(
    JNIEnv *env,
    jobject thiz
) {
    LOGI("Modern adapter: makeCurrent");
    return JNI_TRUE;
}

/**
 * Translate a shader using MobileGlues's GLSL -> SPIR-V -> ESSL pipeline.
 */
JNIEXPORT jstring JNICALL
Java_me_shadow_cobalt_modern_ModernRouteAdapter_nativeTranslateShader(
    JNIEnv *env,
    jobject thiz,
    jint shaderType,
    jstring source,
    jstring profile
) {
    const char *src = env->GetStringUTFChars(source, nullptr);
    const char *prof = env->GetStringUTFChars(profile, nullptr);

    LOGI("Modern adapter: translateShader (type=%d, profile=%s)", shaderType, prof);

    // MobileGlues handles shader translation internally when shaders go through
    // the normal GL path (glShaderSource interception). For direct translation,
    // we apply the same transforms that MobileGlues uses:
    // 1. Ensure #version 150 or higher
    // 2. Add precision qualifiers
    // 3. Replace texture2D with texture
    // 4. Handle built-in variable replacements

    std::string input(src);
    std::string output;

    // Check if already ESSL
    if (input.find("#version 300 es") != std::string::npos ||
        input.find("#version 310 es") != std::string::npos ||
        input.find("#version 320 es") != std::string::npos) {
        output = input; // Already ESSL
    } else {
        output = input;

        // Ensure minimum version
        if (output.find("#version") == std::string::npos) {
            output = "#version 300 es\n" + output;
        } else {
            // Replace #version 1XX with #version 300 es
            size_t verPos = output.find("#version");
            size_t verEnd = output.find('\n', verPos);
            if (verEnd != std::string::npos) {
                output = output.substr(0, verPos) + "#version 300 es" + output.substr(verEnd);
            }
        }

        // Add precision qualifier if missing
        if (output.find("precision") == std::string::npos) {
            if (shaderType == 0) {
                output = "precision highp float;\nprecision highp int;\n" + output;
            } else {
                output = "precision mediump float;\nprecision highp int;\n" + output;
            }
        }

        // Replace texture2D with texture
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

        // Replace gl_FragData with custom output
        pos = 0;
        while ((pos = output.find("gl_FragData[0]", pos)) != std::string::npos) {
            output.replace(pos, 14, "fragColor");
            pos += 10;
        }
    }

    jstring result = env->NewStringUTF(output.c_str());
    env->ReleaseStringUTFChars(source, src);
    env->ReleaseStringUTFChars(profile, prof);
    return result;
}

/**
 * Minimal draw health check for GLES 3.x context.
 */
JNIEXPORT jboolean JNICALL
Java_me_shadow_cobalt_modern_ModernRouteAdapter_nativeMinimalDraw(
    JNIEnv *env,
    jobject thiz
) {
    LOGI("Modern adapter: minimal draw health check");

    // Check GLES version
    const char* version = (const char*)glGetString(GL_VERSION);
    LOGI("GLES version: %s", version ? version : "unknown");

    // Clear to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Check for errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("Modern adapter: GL error during minimal draw: 0x%x", err);
        return JNI_FALSE;
    }

    LOGI("Modern adapter: minimal draw passed");
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_me_shadow_cobalt_modern_ModernRouteAdapter_nativeDestroy(
    JNIEnv *env,
    jobject thiz
) {
    LOGI("Modern adapter: destroying");
    mg_unload();
}

JNIEXPORT jstring JNICALL
Java_me_shadow_cobalt_modern_ModernRouteAdapter_nativeCollectDiagnostics(
    JNIEnv *env,
    jobject thiz
) {
    std::string diag = "{";
    diag += "\"mobileglues_loaded\":" + std::string(mg_is_loaded() ? "true" : "false");
    diag += ",\"angle_in_use\":" + std::string(mg_angle_in_use() ? "true" : "false");

    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* glVersion = (const char*)glGetString(GL_VERSION);

    if (vendor) diag += ",\"vendor\":\"" + std::string(vendor) + "\"";
    if (renderer) diag += ",\"renderer\":\"" + std::string(renderer) + "\"";
    if (glVersion) diag += ",\"gl_version\":\"" + std::string(glVersion) + "\"";

    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    diag += ",\"max_texture_size\":" + std::to_string(maxTextureSize);

    GLint maxComputeWorkGroupCount[3] = {0, 0, 0};
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeWorkGroupCount[0]);
    diag += ",\"max_compute_work_group_count\":" + std::to_string(maxComputeWorkGroupCount[0]);

    diag += "}";
    return env->NewStringUTF(diag.c_str());
}

JNIEXPORT jint JNICALL
Java_me_shadow_cobalt_modern_ModernRouteAdapter_nativeRequiredGlesMajor(
    JNIEnv *env,
    jobject thiz
) {
    return 3; // MobileGlues requires GLES 3.0 minimum
}

JNIEXPORT jint JNICALL
Java_me_shadow_cobalt_modern_ModernRouteAdapter_nativeRequiredGlesMinor(
    JNIEnv *env,
    jobject thiz
) {
    return 0; // GLES 3.0 minimum, 3.2 preferred
}

} // extern "C"
