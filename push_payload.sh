#!/bin/bash

PACKAGE=com.and.games505.TerrariaPaid

rm payload.so
./jni/payload/build_payload.sh

adb push payload.so /sdcard/Android/data/$PACKAGE/files/payload.so

echo "cp /sdcard/Android/data/$PACKAGE/files/payload.so /data/data/$PACKAGE/files/payload.so" | sudo waydroid shell

adb shell am force-stop $PACKAGE
sleep 1
adb shell am start -n $PACKAGE/com.unity3d.player.UnityPlayerActivity

echo "Done! Watching logs..."
adb logcat --clear
adb logcat | grep -E "Pure|Payload"