cmake_minimum_required(VERSION 3.22.1)
project("cobalt" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ===================== Compiler flags =====================
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fms-extensions -std=gnu2x -O3 -fno-exceptions -ffast-math")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions -std=gnu++20 -O3 -ffast-math -fno-rtti")

add_definitions(-DANDROID)
add_definitions(-DHAVE_STRUCT_TIMESPEC)
add_definitions(-DDETECT_OS_ANDROID)
add_definitions(-DHAVE_OPENGL)
add_definitions(-DHAVE_OPENGL_ES_1)
add_definitions(-DHAVE_OPENGL_ES_2)

# ===================== 3rdparty: glslang & SPIRV-Cross =====================
set(ENABLE_RTTI                ON  CACHE BOOL "Enable RTTI for glslang")
set(ENABLE_HLSL                OFF CACHE BOOL "Disable HLSL")
set(BUILD_EXTERNAL             OFF CACHE BOOL "Disable build external deps")
set(ENABLE_GLSLANG_BINARIES    OFF CACHE BOOL "Disable glslangValidator")
set(ENABLE_OPT                 OFF CACHE BOOL "Disable SPIRV-Tools opt")
set(SKIP_GLSLANG_INSTALL       ON  CACHE BOOL "Skip glslang install")
set(SKIP_SPIRV_CROSS_INSTALL   ON  CACHE BOOL "Skip SPIRV-Cross install")

set(SPIRV_CROSS_ENABLE_C_API   ON  CACHE BOOL "Enable C API")
set(SPIRV_CROSS_ENABLE_GLSL    ON  CACHE BOOL "Enable GLSL backend")
set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "Disable reflect")
set(SPIRV_CROSS_ENABLE_HLSL    OFF CACHE BOOL "Disable HLSL backend")
set(SPIRV_CROSS_ENABLE_MSL     OFF CACHE BOOL "Disable MSL backend")
set(SPIRV_CROSS_ENABLE_CPP     OFF CACHE BOOL "Disable C++ API target")
set(SPIRV_CROSS_CLI            OFF CACHE BOOL "Disable CLI binary")
set(SPIRV_CROSS_STATIC         ON  CACHE BOOL "Prefer static libs")
set(SPIRV_CROSS_ENABLE_UTIL    OFF CACHE BOOL "Disable util")

add_subdirectory(3rdparty/glslang)
add_subdirectory(3rdparty/SPIRV-Cross)

# ===================== Shared include paths =====================
set(COBALT_COMMON_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GLES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR
    ${CMAKE_CURRENT_SOURCE_DIR}/core/shared
)

# ===================== LIB 1: cobalt_legacy (gl4es engine) =====================
# Handles Minecraft <=1.17 — desktop GL 1.x/2.x → GLES translation
set(LEGACY_NAME cobalt_legacy)

file(GLOB LEGACY_GL_SRC       core/gl/*.c)
file(GLOB LEGACY_AGL_SRC      core/agl/*.c)
file(GLOB LEGACY_EGL_SRC      core/egl/*.c)
file(GLOB LEGACY_GLX_SRC      core/glx/*.c)
file(GLOB LEGACY_MATH_SRC     core/math/*.c)
file(GLOB LEGACY_VGPU_SRC     core/vgpu/*.c)
file(GLOB LEGACY_WRAP_SRC     core/wrap/*.c)
file(GLOB LEGACY_IPROTECC_SRC core/iprotecc/*.c)

# GLSL optimizer (Mesa-based)
file(GLOB_RECURSE LEGACY_OPT_SRC
    glsl_optimizer/src/code/*.cpp
    glsl_optimizer/src/compiler/glsl/*.cpp
    glsl_optimizer/src/compiler/glsl/*.c
    glsl_optimizer/src/compiler/nir/*.c
    glsl_optimizer/src/gallium/auxiliary/util/*.c
    glsl_optimizer/src/mapi/glapi/*.c
    glsl_optimizer/src/mesa/main/*.c
    glsl_optimizer/src/mesa/math/*.c
    glsl_optimizer/src/mesa/program/*.c
    glsl_optimizer/src/mesa/vbo/*.c
    glsl_optimizer/src/util/format/*.c
    glsl_optimizer/src/util/sha1/*.c
)

set(LEGACY_INCLUDES
    ${COBALT_COMMON_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl/iProtecc
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl/math
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl/vgpu
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl/wrap
    ${CMAKE_CURRENT_SOURCE_DIR}/core/egl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/glx
    ${CMAKE_CURRENT_SOURCE_DIR}/core/agl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/wrap
    # GLSL optimizer
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/include/c11
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/include/GL
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/include/GL/KHR
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/include/optimizer
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/code
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/compiler/glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/compiler/glsl/glcpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/compiler/nir
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/gallium
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/gallium/auxiliary
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/gallium/auxiliary/util
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/gallium/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/gallium/include/frontend
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/gallium/include/pipe
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/mapi
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/mapi/glapi
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/mesa/main
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/mesa/math
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/mesa/program
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/mesa/vbo
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/util
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/util/format
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl_optimizer/src/util/sha1
)

add_library(${LEGACY_NAME} SHARED
    ${LEGACY_GL_SRC}
    ${LEGACY_AGL_SRC}
    ${LEGACY_EGL_SRC}
    ${LEGACY_GLX_SRC}
    ${LEGACY_MATH_SRC}
    ${LEGACY_VGPU_SRC}
    ${LEGACY_WRAP_SRC}
    ${LEGACY_IPROTECC_SRC}
    ${LEGACY_OPT_SRC}
)

target_include_directories(${LEGACY_NAME} PRIVATE ${LEGACY_INCLUDES})

target_link_libraries(${LEGACY_NAME} PRIVATE
    android log EGL GLESv2 GLESv3 z m dl
)

target_link_options(${LEGACY_NAME} PRIVATE -Wl,-Bsymbolic-functions)

# Suppress warnings in optimizer code
foreach(opt_file ${LEGACY_OPT_SRC})
    set_source_files_properties(${opt_file} PROPERTIES
        COMPILE_FLAGS "-w -fpermissive -Wno-everything"
    )
endforeach()

# ===================== LIB 2: cobalt_modern (MobileGlues engine) =====================
# Handles Minecraft >1.17 — desktop GL 3.x/4.x → GLES via SPIR-V pipeline
set(MODERN_NAME cobalt_modern)

file(GLOB MODERN_GL_SRC     core/modern/gl/*.cpp)
file(GLOB MODERN_EGL_SRC    core/modern/egl/*.cpp)
file(GLOB MODERN_GLES_SRC   core/modern/gles/*.cpp)
file(GLOB MODERN_GLSL_SRC   core/modern/glsl/*.cpp)
file(GLOB MODERN_CFG_SRC    core/modern/config/*.cpp core/modern/config/*.c)
file(GLOB MODERN_FSR_SRC    core/modern/fsr/*.cpp)
file(GLOB MODERN_TOP_SRC    core/modern/*.cpp)
file(GLOB MODERN_SHARED_SRC core/shared/*.c core/shared/*.cpp)

set(MODERN_INCLUDES
    ${COBALT_COMMON_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/gl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/egl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/gles
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/config
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/fsr
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SPIRV-Cross
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/xxhash
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/flat_hash_map
)

add_library(${MODERN_NAME} SHARED
    ${MODERN_GL_SRC}
    ${MODERN_EGL_SRC}
    ${MODERN_GLES_SRC}
    ${MODERN_GLSL_SRC}
    ${MODERN_CFG_SRC}
    ${MODERN_FSR_SRC}
    ${MODERN_TOP_SRC}
    ${MODERN_SHARED_SRC}
)

target_include_directories(${MODERN_NAME} PRIVATE ${MODERN_INCLUDES})

target_link_libraries(${MODERN_NAME} PRIVATE
    glslang::glslang
    glslang::glslang-default-resource-limits
    SPIRV-Cross
    android log EGL GLESv2 GLESv3 z m dl
)

target_link_options(${MODERN_NAME} PRIVATE -Wl,-Bsymbolic-functions)

# ===================== LIB 3: cobalt (JNI bridge) =====================
# Loaded by Kotlin via System.loadLibrary("cobalt"). dlopen's the right engine.
set(BRIDGE_NAME cobalt)

add_library(${BRIDGE_NAME} SHARED jni/cobalt_jni.cpp)

target_include_directories(${BRIDGE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${BRIDGE_NAME} PRIVATE
    android log dl
)

# ===================== Status =====================
message(STATUS "Cobalt: Building ${LEGACY_NAME} + ${MODERN_NAME} + ${BRIDGE_NAME}")
