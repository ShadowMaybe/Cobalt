cmake_minimum_required(VERSION 3.22.1)
project("cobalt-modern" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Modern route adapter native bridge
# Integrates with MobileGlues for Minecraft > 1.17.0
# Produces libcobalt-modern.so which wraps the MobileGlues GLES 3.x layer

add_library(cobalt-modern SHARED
    modern_bridge.cpp
    mg_loader.cpp
)

target_include_directories(cobalt-modern PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cobalt-modern
    android
    log
    dl
    EGL
    GLESv3
)
