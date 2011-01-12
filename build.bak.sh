#!/bin/bash

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4b
# Set environment to CrystaX NDK with RTTI and exceptions instead of original NDK
# export PATH=$PATH:~/src/endless_space/android-ndk-r4-crystax/ndk-build

cd project && nice -n5 ndk-build -j2 V=1 && ant release&& cd bin && jarsigner -verbose -keystore android_key DemoActivity-unsigned.apk xianle &&rm -rf temp.apk &&zipalign -v 4 DemoActivity-unsigned.apk temp.apk && adb install -r temp.apk

