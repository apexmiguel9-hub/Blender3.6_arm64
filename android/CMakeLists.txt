cmake_minimum_required(VERSION 3.18)
project(blender_android_wrapper LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build the JNI wrapper that bridges Blender <-> Android
add_library(blender_jni SHARED
    jni/blender_jni.cpp
)

target_include_directories(blender_jni PRIVATE
    ${CMAKE_SOURCE_DIR}/../blender279/source/blender/windowmanager
    ${CMAKE_SOURCE_DIR}/../blender279/intern/ghost
)

target_link_libraries(blender_jni
    EGL
    GLESv2
    android
    log
)

# In production, we'd link against libblender.so
# For now, it's loaded at runtime via System.loadLibrary()
