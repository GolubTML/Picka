#!/bin/bash

rm /mnt/hdd/Terraria-mobile/Terraria-ml.apk
rm /mnt/hdd/Terraria-mobile/Terraria-ml-full.apk

echo "Assemble base apk"
apktool b /mnt/hdd/Terraria-mobile/Terraria-v1.4.5.6.4-full/ -o /mnt/hdd/Terraria-mobile/Terraria-ml.apk

echo "Aligned apk"
zipalign -v 4 /mnt/hdd/Terraria-mobile/Terraria-ml.apk /mnt/hdd/Terraria-mobile/Terraria-ml-full.apk

echo "Sign apk"
apksigner sign --ks /mnt/hdd/Terraria-mobile/my-release-key.jks /mnt/hdd/Terraria-mobile/Terraria-ml-full.apk

echo "Install apk to emulator"
adb install -r /mnt/hdd/Terraria-mobile/Terraria-ml-full.apk