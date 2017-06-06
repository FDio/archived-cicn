#!/bin/bash

cd MetisForwarder
if [ ! -f local.properties ]; then
	echo sdk.dir=${SDK} > local.properties
	echo ndk.dir=${NDK} >> local.properties
fi

if [ "$1" = "DEBUG" ]; then
	./gradlew assembleDebug
else
	./gradlew assembleRelease
fi

echo "Apks are inside MetisForwarder/app/build/outputs/apk"
cd ..
