cmake_minimum_required(VERSION 3.22.1)
project("cobalt" LANGUAGES C CXX)
enable_language(CXX)
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
set(ENABLE_HLSL                OFF CACHE BOOL "Disable HLSL input for glslang")
set(BUILD_EXTERNAL             OFF CACHE BOOL "Disable build external deps")
set(ENABLE_GLSLANG_BINARIES    OFF CACHE BOOL "Disable glslangValidator")
set(ENABLE_OPT                 OFF CACHE BOOL "Disable SPIRV-Tools opt")
set(SKIP_GLSLANG_INSTALL       ON  CACHE BOOL "Skip glslang install")
set(SKIP_SPIRV_CROSS_INSTALL   ON  CACHE BOOL "Skip SPIRV-Cross install")

set(SPIRV_CROSS_ENABLE_C_API   ON  CACHE BOOL "Enable C API for SPIRV-Cross")
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

# ===================== Cobalt core sources =====================

# --- Legacy path (gl4es GL translation engine) ---
file(GLOB COBALT_GL_SRC       core/gl/*.c)
file(GLOB COBALT_AGL_SRC      core/agl/*.c)
file(GLOB COBALT_EGL_SRC      core/egl/*.c)
file(GLOB COBALT_GLX_SRC      core/glx/*.c)
file(GLOB COBALT_MATH_SRC     core/math/*.c)
file(GLOB COBALT_VGPU_SRC     core/vgpu/*.c)
file(GLOB COBALT_WRAP_SRC     core/wrap/*.c)
file(GLOB COBALT_IPROTECC_SRC core/iprotecc/*.c)

# --- GLSL optimizer (Mesa-based, used by legacy path) ---
file(GLOB_RECURSE COBALT_OPT_SRC
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

# --- Modern path (MobileGlues GL implementation + SPIR-V shader pipeline) ---
file(GLOB COBALT_MODERN_GL_SRC    core/modern/gl/*.cpp)
file(GLOB COBALT_MODERN_EGL_SRC   core/modern/egl/*.cpp)
file(GLOB COBALT_MODERN_GLES_SRC  core/modern/gles/*.cpp)
file(GLOB COBALT_MODERN_GLSL_SRC  core/modern/glsl/*.cpp)
file(GLOB COBALT_MODERN_CFG_SRC   core/modern/config/*.cpp core/modern/config/*.c)
file(GLOB COBALT_MODERN_FSR_SRC   core/modern/fsr/*.cpp)
file(GLOB COBALT_MODERN_TOP_SRC   core/modern/*.cpp)

# --- Shared (cJSON config, getter) ---
file(GLOB COBALT_SHARED_SRC    core/shared/*.c core/shared/*.cpp)

# ===================== Collect all sources =====================
set(COBALT_ALL_SRC
    # Legacy engine
    ${COBALT_GL_SRC}
    ${COBALT_AGL_SRC}
    ${COBALT_EGL_SRC}
    ${COBALT_GLX_SRC}
    ${COBALT_MATH_SRC}
    ${COBALT_VGPU_SRC}
    ${COBALT_WRAP_SRC}
    ${COBALT_IPROTECC_SRC}
    # GLSL optimizer
    ${COBALT_OPT_SRC}
    # Modern engine
    ${COBALT_MODERN_GL_SRC}
    ${COBALT_MODERN_EGL_SRC}
    ${COBALT_MODERN_GLES_SRC}
    ${COBALT_MODERN_GLSL_SRC}
    ${COBALT_MODERN_CFG_SRC}
    ${COBALT_MODERN_FSR_SRC}
    ${COBALT_MODERN_TOP_SRC}
    # Shared
    ${COBALT_SHARED_SRC}
)

# ===================== Include directories =====================
set(COBALT_INCLUDES
    # Legacy engine includes
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl/iProtecc
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl/math
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl/vgpu
    ${CMAKE_CURRENT_SOURCE_DIR}/core/gl/wrap
    ${CMAKE_CURRENT_SOURCE_DIR}/core/egl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/glx
    ${CMAKE_CURRENT_SOURCE_DIR}/core/agl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/wrap
    ${CMAKE_CURRENT_SOURCE_DIR}/core/shared
    # Public headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/EGL
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GL/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GLES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR
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
    # Modern engine includes
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/gl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/egl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/gles
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/config
    ${CMAKE_CURRENT_SOURCE_DIR}/core/modern/fsr
    # 3rdparty includes (for MobileGlues' use of glslang/SPIRV-Cross headers)
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SPIRV-Cross
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/xxhash
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/flat_hash_map
)

# ===================== Build shared library =====================
add_library(${CMAKE_PROJECT_NAME} SHARED ${COBALT_ALL_SRC})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${COBALT_INCLUDES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    glslang::glslang
    glslang::glslang-default-resource-limits
    SPIRV-Cross
    android
    log
    EGL
    GLESv2
    GLESv3
    z
    m
    dl
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -Wl,-Bsymbolic-functions)

# Per-file compile options for problematic files
foreach(opt_file ${COBALT_OPT_SRC})
    set_source_files_properties(${opt_file} PROPERTIES
        COMPILE_FLAGS "-w -fpermissive -Wno-everything"
    )
endforeach()

message(STATUS "Cobalt: Building ${CMAKE_PROJECT_NAME} with ${CMAKE_C_STANDARD} / ${CMAKE_CXX_STANDARD}")
