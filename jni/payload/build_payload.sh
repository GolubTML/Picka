#!/bin/bash

NDK=/opt/android-ndk
CLANG=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android26-clang
CLANGPP=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android26-clang++

$CLANGPP -shared -fPIC -O2 -DIMGUI_IMPL_OPENGL_ES3 \
    -I jni/imgui \
    -I jni/imgui/backends \
    -o payload.so \
    jni/payload/*.cpp \
    jni/payload/test_mods/*.cpp \
    jni/payload/SDK/*.cpp \
    jni/imgui/imgui*.cpp \
    jni/imgui/backends/imgui_impl_opengl3.cpp \
    -I. \
    -Ijni/payload \
    -Ijni/payload/test_mods \
    -llog \
    -lEGL \
    -lGLESv3 \
    -static-libstdc++

echo "Done: payload.so"