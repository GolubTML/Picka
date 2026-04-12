#!/bin/bash

NDK=/opt/android-ndk
CLANG=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android26-clang

$CLANG \
    -shared \
    -fPIC \
    -o libloader.so \
    jni/loader/main.c \
    -llog

echo "Done: libloader.so"