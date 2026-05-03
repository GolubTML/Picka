#!/bin/bash

rm JavaHelper/classes.dex
rm JavaHelper/classes2.dex
rm JavaHelper/FloatButtonHelper.class
rm JavaHelper/FloatButtonHelper$1.class

javac -source 1.8 -target 1.8 -cp "/opt/android-sdk/platforms/android-33/android.jar" JavaHelper/FloatButtonHelper.java

d8 JavaHelper/FloatButtonHelper.class \
    --lib "/opt/android-sdk/platforms/android-33/android.jar" \
    --output JavaHelper/