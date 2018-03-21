 #############################################################################
 # Copyright (c) 2017 Cisco and/or its affiliates.
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at:
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 ##############################################################################

#!/bin/bash

set -ex

if [ "$ANDROID_ARCH" = "arm" ]; then
	TOOLCHAIN=`pwd`/sdk/toolchain
	BASE_PATH=`pwd`
	mkdir -p qt
	cd qt
	export QT_HOME=`pwd`/Qt
	if [ ! -d ${QT_HOME} ]; then
		if [ $OS = "darwin" ]; then
			if [ ! -f qt-opensource-mac-x64-android-ios-5.8.0.dmg ]; then
				wget http://download.qt.io/archive/qt/5.8/5.8.0/qt-opensource-mac-x64-android-ios-5.8.0.dmg
			fi
			VOLUME=$(hdiutil attach qt-opensource-mac-x64-android-ios-5.8.0.dmg | tail -1 | awk '{print $3}')
			$VOLUME/qt-opensource-mac-x64-android-ios-5.8.0.app/Contents/MacOS/qt-opensource-mac-x64-android-ios-5.8.0  --script ../scripts/install_script.sh -platform minimal --verbose
			diskutil unmount $VOLUME
		else
			if [ ! -f qt-opensource-linux-x64-android-5.8.0.run ]; then
				wget http://download.qt.io/archive/qt/5.8/5.8.0/qt-opensource-linux-x64-android-5.8.0.run
			fi
			chmod +x qt-opensource-linux-x64-android-5.8.0.run
			./qt-opensource-linux-x64-android-5.8.0.run --script ../scripts/install_script.sh -platform minimal --verbose
		fi
	fi


	if [ ! -d ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/boost ]; then
		ln -s $DISTILLERY_INSTALL_DIR/include/ccnx  ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/
		ln -s $DISTILLERY_INSTALL_DIR/include/boost  ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/
		ln -s $DISTILLERY_INSTALL_DIR/include/parc  ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/
		ln -s $DISTILLERY_INSTALL_DIR/include/LongBow  ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/
		ln -s $DISTILLERY_INSTALL_DIR/include/icnet  ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/
		ln -s $DISTILLERY_INSTALL_DIR/include/dash  ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/
	fi

	echo "clone and compile ffmpeg"
	if [ ! -f $DISTILLERY_INSTALL_DIR/lib/libavcodec.so -o ! -f $DISTILLERY_INSTALL_DIR/libavfilter.so -o ! -f $DISTILLERY_INSTALL_DIR/lib/libavformat.so -o ! -f $DISTILLERY_INSTALL_DIR/lib/libavutil.so -o ! -f $DISTILLERY_INSTALL_DIR/lib/libswresample.so -o ! -f $DISTILLERY_INSTALL_DIR/lib/libswscale.so ]; then

		if [ ! -d ffmpeg ]; then
			git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
		fi
	fi

	export FFSRC=`pwd`/ffmpeg
	export ANDROID_NDK=${NDK}
	export ANDROID_HOME=${SDK}
	export ANDROID_NDK_HOST=${OS}-${ARCH}
	export ANDROID_NDK_PLATFORM=android-26
	export ANDROID_NDK_ROOT=${NDK}
	export ANDROID_NDK_TOOLCHAIN_PREFIX=arm-linux-androideabi
	export ANDROID_NDK_TOOLCHAIN_VERSION=4.9
	export ANDROID_NDK_TOOLS_PREFIX=arm-linux-androideabi
	export ANDROID_SDK_ROOT=${SDK}
	export ANDROID_API_VERSION=${ANDROID_PLATFORM}
	export PATH=$PATH:${ANDROID_HOME}/tools:${JAVA_HOME}/bin
	if [ ! -d ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/QtAV ]; then
		if [ ! -d QtAV ]; then
        		git clone https://github.com/wang-bin/QtAV.git
    		fi
		cd QtAV
		echo "INCLUDEPATH += ${DISTILLERY_INSTALL_DIR}/include" >> .qmake.conf
		echo "LIBS += -L${DISTILLERY_INSTALL_DIR}/lib" >> .qmake.conf
		git submodule update --init
		cd tools/build_ffmpeg
		./avbuild.sh android armv7
		cp sdk-android-gcc/lib/armeabi-v7a/lib* ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/lib/
		cp -r sdk-android-gcc/include/* ${QT_HOME}/5.8/android_${ANDROID_ARCH}v7/include/
		cd ../..
		mkdir -p ${DISTILLERY_BUILD_DIR}/qtav
		cd ${DISTILLERY_BUILD_DIR}/qtav
		${QT_HOME}/5.8/android_armv7/bin/qmake -r -spec android-g++ $BASE_PATH/qt/QtAV/QtAV.pro
		make
		make install INSTALL_ROOT=android
		sh sdk_install.sh
	fi
	cd ${DISTILLERY_ROOT_DIR}
fi