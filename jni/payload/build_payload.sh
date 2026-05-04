#!/bin/bash

NDK=/opt/android-ndk
CLANG=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android26-clang
CLANGPP=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android26-clang++


LUA_DIR="./jni/payload/libs/Lua54"
LUA_SRCS="$LUA_DIR/*.c"
CPP_SRCS=$(find jni/payload -name "*.cpp")

$CLANGPP -shared -fPIC -O2 \
    -o payload.so \
    -x c $LUA_SRCS \
    -x c++ $CPP_SRCS \
    -I. \
    -I$LUA_DIR \
    -Ijni/payload \
    -Ijni/payload/test_mods \
    -llog -landroid -ldl \
    -static-libstdc++ \
    -Wl,--export-dynamic \
    -Wl,--no-undefined

if [ $? -eq 0 ]; then
    echo "Done: payload.so"
else
    echo "Compilation FAILED!"
    exit 1
fi