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

set -e

if [ $ARCH = "x86" ]; then
	echo "Qt is not available for x86 systems"
	exit 1
fi


mkdir -p qt
cd qt
export QT_HOME=`pwd`/Qt
if [ ! -d ${QT_HOME} ]; then
	if [ $OS = "darwin" ]; then
		if [ ! -f qt-opensource-mac-x64-android-5.7.1.dmg ]; then
			wget http://download.qt.io/archive/qt/5.7/5.7.1/qt-opensource-mac-x64-android-5.7.1.dmg
		fi
		
		VOLUME=$(hdiutil attach qt-opensource-mac-x64-android-5.7.1.dmg | tail -1 | awk '{print $3}')
		$VOLUME/qt-opensource-mac-x64-android-5.7.1.app/Contents/MacOS/qt-opensource-mac-x64-android-5.7.1  --script ../scripts/install_script.sh -platform minimal --verbose
		diskutil unmount $VOLUME
	else
		if [ ! -f qt-opensource-linux-x64-android-5.7.1.run ]; then
			wget http://download.qt.io/archive/qt/5.7/5.7.1/qt-opensource-linux-x64-android-5.7.1.run
		fi
		chmod +x qt-opensource-linux-x64-android-5.7.1.run
		./qt-opensource-linux-x64-android-5.7.1.run --script ../scripts/install_script.sh -platform minimal --verbose
	fi
fi

cp -f $DISTILLERY_INSTALL_DIR/lib/libdash.so ${QT_HOME}/5.7/android_${ANDROID_ARCH}/lib/

if [ ! -d ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/boost ]; then
	ln -s $DISTILLERY_INSTALL_DIR/include/ccnx  ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/
	ln -s $DISTILLERY_INSTALL_DIR/include/boost  ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/
	ln -s $DISTILLERY_INSTALL_DIR/include/parc  ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/
	ln -s $DISTILLERY_INSTALL_DIR/include/LongBow  ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/
	ln -s $DISTILLERY_INSTALL_DIR/include/icnet  ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/
	ln -s $DISTILLERY_INSTALL_DIR/include/dash  ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/
fi

if [[ ! -f ${QT_HOME}/5.7/android_${ANDROID_ARCH}/lib/libavformat.so || ! -f ${QT_HOME}/5.7/android_${ANDROID_ARCH}/lib/libavfilter.so || ! -f ${QT_HOME}/5.7/android_${ANDROID_ARCH}/lib/libavformat.so || ! -f ${QT_HOME}/5.7/android_${ANDROID_ARCH}/lib/libavutil.so || ! -f ${QT_HOME}/5.7/android_${ANDROID_ARCH}/lib/libswresample.so || ! -f ${QT_HOME}/5.7/android_${ANDROID_ARCH}/lib/libswscale.so ]]; then
	if [ ! -f ffmpeg-3.1.4-android.7z ]; then
		wget https://downloads.sourceforge.net/project/qtav/depends/FFmpeg/android/ffmpeg-3.1.4-android.7z	
	fi
	7z x ffmpeg-3.1.4-android.7z -offmpeg
	cp ffmpeg/ffmpeg-3.1.4-android-armv7a/lib/lib* ${QT_HOME}/5.7/android_${ANDROID_ARCH}/lib/
	cp -r ffmpeg/ffmpeg-3.1.4-android-armv7a/include/* ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/
fi

export ANDROID_HOME=${SDK}
export ANDROID_NDK_HOST=${OS}-${ARCH}
export ANDROID_NDK_PLATFORM=${ANDROID_PLATFORM}
export ANDROID_NDK_ROOT=${NDK}
export ANDROID_NDK_TOOLCHAIN_PREFIX=arm-linux-androideabi  
export ANDROID_NDK_TOOLCHAIN_VERSION=4.9
export ANDROID_NDK_TOOLS_PREFIX=arm-linux-androideabi  
export ANDROID_SDK_ROOT=${SDK} 
export ANDROID_API_VERSION=${ANDROID_PLATFORM}
export PATH=$PATH:${ANDROID_HOME}/tools:${JAVA_HOME}/bin
if [ ! -d ${QT_HOME}/5.7/android_${ANDROID_ARCH}/include/QtAV ]; then
	git clone https://github.com/wang-bin/QtAV.git 
	cd QtAV
	mkdir -p ${DISTILLERY_BUILD_DIR}/qtav
	cd ${DISTILLERY_BUILD_DIR}/qtav
	${QT_HOME}/5.7/android_${ANDROID_ARCH}/bin/qmake -r -spec android-g++ ${DISTILLERY_ROOT_DIR}/qt/QtAV/QtAV.pro
	make
	make install INSTALL_ROOT=android
	sh sdk_install.sh
fi
cd ${DISTILLERY_ROOT_DIR}