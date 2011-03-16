AppFullName=com.xianle.doomtnt
AppVersionCode=10
AppVersionName="1.0 releae TNT of Doom"
AppName="TNT Doom"
echo Patching java file
for F in project/src/*.java; do
	echo Patching $F
	cat $F | \
		sed "s/package .*;/package com.xianle.doomtnt;/" > \
		$F.1
	mv -f $F.1 $F
done
echo Patching project/AndroidManifest.xml
cat project/AndroidManifest.xml | \
	sed "s/package=.*/package=\"$AppFullName\"/" | \
	sed "s^android:versionCode=.*^android:versionCode=\"$AppVersionCode\"^" | \
	sed "s^android:versionName=.*^android:versionName=\"$AppVersionName\"^" > \
	project/AndroidManifest.xml.1
mv -f project/AndroidManifest.xml.1 project/AndroidManifest.xml

echo Pathching project/res/layout/cover.xml
cat project/res/layout/cover.xml | \
	sed "s^xmlns:myapp=.*^xmlns:myapp=\"http://schemas.android.com/apk/res/\"$AppFullName\"\"^">\
	project/res/layout/cover.xml.1
mv -f project/res/layout/cover.xml.1 project/res/layout/cover.xml

AppShortName=`echo $AppName | sed 's/ //g'`
DataPath="$AppFullName"
AppFullNameUnderscored=`echo $AppFullName | sed 's/[.]/_/g'`
AppSharedLibrariesPath=/data/data/$AppFullName/lib

echo Patching project/jni/Android.mk
cat project/jni/Android.mk | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" > \
	project/jni/Android.mk.1
if [ -n "`diff -w project/jni/Android.mk.1 project/jni/Android.mk`" ] ; then
	mv -f project/jni/Android.mk.1 project/jni/Android.mk
else
	rm -rf project/jni/Android.mk.1
fi

echo Patching project/res/values/strings.xml
cat project/res/values/strings.xml | \
	sed "s^[<]string name=\"app_name\"[>].*^<string name=\"app_name\">$AppName</string>^" > \
	project/res/values/strings.xml.1
mv -f project/res/values/strings.xml.1 project/res/values/strings.xml

echo Forcing rebuild of specific files
rm -rf project/libs/*
for OUT in obj; do
rm -rf project/$OUT/local/*/objs/sdl_main/* project/$OUT/local/*/libsdl_main.so
rm -rf project/$OUT/local/*/libsdl.so
rm -rf project/$OUT/local/*/libstlport.a # Should be re-linked if you're changing toolchain
rm -rf project/$OUT/local/*/objs/sdl/src/*/android
rm -rf project/$OUT/local/*/objs/sdl/src/video/SDL_video.o
rm -rf project/$OUT/local/*/objs/sdl/SDL_renderer_gles.o
# Do not rebuild libraries that do not need that
find project/$OUT/local -name "*.[oa]" -exec touch '{}' \;
done