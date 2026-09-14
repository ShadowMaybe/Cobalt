cmake_minimum_required(VERSION 3.22.1)
project("cobalt-legacy" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Legacy route adapter native bridge
# Integrates with gl4es_extra_extra for Minecraft <= 1.17.0
# Produces libcobalt-legacy.so which wraps the gl4es compatibility layer

add_library(cobalt-legacy SHARED
    legacy_bridge.cpp
    gl4es_loader.cpp
)

target_include_directories(cobalt-legacy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cobalt-legacy
    android
    log
    dl
    EGL
    GLESv2
)
