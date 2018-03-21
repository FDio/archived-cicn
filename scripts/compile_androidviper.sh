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


#!/bin/bash
set -e
ANDROID_ARCH=armv7
export ANDROID_HOME=${SDK}
export ANDROID_NDK_HOST=${OS}-${ARCH}
export ANDROID_NDK_PLATFORM=android-23
export ANDROID_NDK_ROOT=${NDK}
export ANDROID_NDK_TOOLCHAIN_PREFIX=arm-linux-androideabi
export ANDROID_NDK_TOOLCHAIN_VERSION=4.9
export ANDROID_NDK_TOOLS_PREFIX=arm-linux-androideabi
export ANDROID_SDK_ROOT=${SDK}
export ANDROID_API_VERSION=android-23
export PATH=$PATH:${ANDROID_HOME}/tools:${JAVA_HOME}/bin
echo $QT_HOME
cd ${DISTILLERY_ROOT_DIR}
export DISTILLARY_INSTALLATION_PATH=${DISTILLERY_ROOT_DIR}/usr_armv7-a
mkdir -p ${DISTILLERY_BUILD_DIR}/viper
cd ${DISTILLERY_BUILD_DIR}/viper
${QT_HOME}/5.8/android_${ANDROID_ARCH}/bin/qmake -r -spec android-g++ ${DISTILLERY_ROOT_DIR}/src/viper/viper.pro
make
make install INSTALL_ROOT=viper-${ANDROID_ARCH}
if [ "$1" = "DEBUG" ]; then
	${QT_HOME}/5.8/android_${ANDROID_ARCH}/bin/androiddeployqt --output viper-${ANDROID_ARCH} --verbose --input android-libviper.so-deployment-settings.json --gradle --android-platform android-23 --stacktrace --debug --target android-23 --debug --sign ${DISTILLERY_ROOT_DIR}/src/viper/android/viper.keystore viper --storepass icn_viper
else
	${QT_HOME}/5.8/android_${ANDROID_ARCH}/bin/androiddeployqt --output viper-${ANDROID_ARCH} --verbose --input android-libviper.so-deployment-settings.json --gradle --android-platform android-23 --stacktrace --debug --target android-23 --release --sign ${DISTILLERY_ROOT_DIR}/src/viper/android/viper.keystore viper --storepass icn_viper
fi
cd ..
