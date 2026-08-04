#!/bin/bash

PACKAGE=com.and.games505.TerrariaPaid

make -C jni/payload -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

adb push build/payload/libpayload.so /sdcard/Android/data/$PACKAGE/files/libpayload.so

echo "cp /sdcard/Android/data/$PACKAGE/files/libpayload.so /data/data/$PACKAGE/files/libpayload.so" | sudo waydroid shell

adb shell am force-stop $PACKAGE
sleep 1
adb shell am start -n $PACKAGE/com.unity3d.player.UnityPlayerActivity

echo "Done! Watching logs..."
adb logcat --clear
adb logcat | grep -E "Pure|Payload"