// Cobalt — Modern engine common includes

#ifndef COBALT_INCLUDES_H
#define COBALT_INCLUDES_H

#define COBALT_RENDERER_NAME "Cobalt"

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <dlfcn.h>

#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <MG/extensions.h>

#include "egl/egl.h"
#include "egl/loader.h"

#ifdef __cplusplus
extern "C" {
#endif

    static int g_initialized = 0;

    void cobalt_proc_init();

#ifdef __cplusplus
}
#endif

#include <flat_hash_map.hpp>

template <typename Key, typename T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
          class Allocator = std::allocator<std::pair<Key, T>>>
using UnorderedMap = ska::flat_hash_map<Key, T, Hash, KeyEqual, Allocator>;

#endif // COBALT_INCLUDES_H
