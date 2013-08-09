#!/bin/bash

SDK_ROOT_PATH=$(dirname $(dirname $(which android)))
SDK_JAR_PATH="$SDK_ROOT_PATH/platforms/android-17/android.jar"
APK_NAME=XWalkAppTemplate
KEY_STORE="scripts/ant/chromium-debug.keystore"

if [ ! -d out ]
then
mkdir out
fi

if [ ! -f "$SDK_ROOT_PATH/tools/lib/anttasks.jar" ]
then
cp -r $SDK_ROOT_PATH/tools/lib/ant-tasks.jar $SDK_ROOT_PATH/tools/lib/anttasks.jar
fi

if [ ! -f "$SDK_ROOT_PATH/platform-tools/aapt" ]
then
cp -r $SDK_ROOT_PATH/build-tools/android-4.2.2/* $SDK_ROOT_PATH/platform-tools/
fi

python scripts/gyp/ant.py "-DADDITIONAL_RES_DIRS=" "-DADDITIONAL_RES_PACKAGES=" "-DADDITIONAL_R_TEXT_FILES=" "-DANDROID_MANIFEST=app_src/AndroidManifest.xml" "-DANDROID_SDK_JAR=$SDK_JAR_PATH" "-DANDROID_SDK_ROOT=$SDK_ROOT_PATH" "-DANDROID_SDK_VERSION=17" "-DLIBRARY_MANIFEST_PATHS=" "-DOUT_DIR=out" "-DRESOURCE_DIR=app_src/res" "-DSTAMP=codegen.stamp" "-Dbasedir=." -buildfile scripts/ant/apk-codegen.xml
python scripts/gyp/javac.py "--output-dir=out/classes" "--classpath=\"libs/xwalk_app_runtime_activity_java.jar\" \"libs/xwalk_app_runtime_client_java.jar\" $SDK_JAR_PATH" "--src-dirs=app_src/src \"out/gen\"" "--javac-includes=" "--chromium-code=0" "--stamp=compile.stamp"
python scripts/gyp/ant.py "-DADDITIONAL_RES_DIRS=" "-DADDITIONAL_RES_PACKAGES=" "-DADDITIONAL_R_TEXT_FILES=" "-DANDROID_SDK_JAR=$SDK_JAR_PATH" "-DANDROID_SDK_ROOT=$SDK_ROOT_PATH" "-DAPK_NAME=$APK_NAME" "-DAPP_MANIFEST_VERSION_CODE=0" "-DAPP_MANIFEST_VERSION_NAME=Developer Build" "-DASSET_DIR=app_src/assets" "-DCONFIGURATION_NAME=Release" "-DOUT_DIR=out" "-DRESOURCE_DIR=app_src/res" "-DSTAMP=package_resources.stamp" "-Dbasedir=." -buildfile scripts/ant/apk-package-resources.xml
python scripts/gyp/jar.py "--classes-dir=out/classes" "--jar-path=libs/app_apk.jar" "--excluded-classes=" "--stamp=jar.stamp"
# Add ProGuard here if needed
python scripts/gyp/dex.py "--dex-path=out/classes.dex" "--android-sdk-root=$SDK_ROOT_PATH" libs/xwalk_app_runtime_activity_java.dex.jar libs/xwalk_app_runtime_client_java.dex.jar out/classes
python scripts/gyp/ant.py "-DANDROID_SDK_ROOT=$SDK_ROOT_PATH" "-DAPK_NAME=$APK_NAME" "-DCONFIGURATION_NAME=Release" "-DOUT_DIR=out" "-DSOURCE_DIR=app_src/src" "-DUNSIGNED_APK_PATH=out/app-unsigned.apk" "-Dbasedir=." -buildfile scripts/ant/apk-package.xml
python scripts/gyp/finalize_apk.py "--android-sdk-root=$SDK_ROOT_PATH" "--unsigned-apk-path=out/app-unsigned.apk" "--final-apk-path=out/$APK_NAME.apk" "--keystore-path=$KEY_STORE"

cp out/$APK_NAME.apk $APK_NAME.apk
