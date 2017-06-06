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

ABI=$1

INSTALLATION_DIR=$2
OS=`echo $OS | tr '[:upper:]' '[:lower:]'`
BASE_DIR=`pwd`
echo "SDK_PATH ${SDK}"
if [ -z ${SDK_PATH} ]; then
	mkdir -p sdk
	cd sdk
	if [ ! -d sdk ]; then
		if [ $OS = darwin ]; then
			if [ ! -f android-sdk_r24.4.1-macosx.zip ]; then
				wget http://dl.google.com/android/android-sdk_r24.4.1-macosx.zip
			fi
			unzip -q android-sdk_r24.4.1-macosx.zip
			mv android-sdk-macosx sdk
		else
			if [ ! -f android-sdk_r24.4.1-linux.zip ]; then
				wget http://dl.google.com/android/android-sdk_r24.4.1-linux.tgz
			fi
			tar zxf android-sdk_r24.4.1-linux.tgz
			mv android-sdk-linux sdk
		fi
		mkdir -p sdk/licenses
		echo -e "\n8933bad161af4178b1185d1a37fbf41ea5269c55" > "sdk/licenses/android-sdk-license"
		echo -e "\n84831b9409646a918e30573bab4c9c91346d8abd" > "sdk/licenses/android-sdk-preview-license"
		echo "y" | ./sdk/tools/android update sdk --filter platform-tools,build-tools-23.0.2,android-23,extra-android-m2repository,extra-google-m2repository --no-ui --all --force
		echo "y" | ./sdk/tools/android update sdk --filter "android-23" --no-ui --all --forceecho y | ./sdk/tools/android update sdk --no-ui --all --filter build-tools-23.0.2
	fi
	cd ..
fi

if [ -z ${NDK_PATH} ]; then
    mkdir -p sdk
	cd sdk
	if [ ! -d ndk-bundle ]; then
		if [ ! -f android-ndk-r14b-${OS}-${ARCH}.zip ]; then
			wget https://dl.google.com/android/repository/android-ndk-r14b-${OS}-${ARCH}.zip
		fi
		unzip -q android-ndk-r14b-${OS}-${ARCH}.zip
		mv android-ndk-r14b ndk-bundle
	fi
	cd ..
fi

if [ -z ${CMAKE_PATH} ]; then
	mkdir -p sdk
	cd sdk
	if [ ! -d cmake ]; then
		if [ ! -f cmake-3.6.3155560-${OS}-${ARCH}.zip ]; then
			wget https://dl.google.com/android/repository/cmake-3.6.3155560-${OS}-${ARCH}.zip
		fi
		unzip -q cmake-3.6.3155560-${OS}-${ARCH}.zip -d cmake
	fi
	cd ..
fi


mkdir -p src
cd src

if [ ! -d ccnxlibs ]; then
	echo "ccnxlibs not found"
	git clone -b ccnxlibs/master https://gerrit.fd.io/r/cicn ccnxlibs
fi
if [ ! -d cframework ]; then
	echo "cframework not found"
	git clone -b cframework/master https://gerrit.fd.io/r/cicn cframework
fi
if [ ! -d sb-forwarder ]; then
	echo "sb-forwarder not found"
	git clone -b sb-forwarder/master https://gerrit.fd.io/r/cicn sb-forwarder
fi
if [ ! -d libicnet ]; then
	echo "libicnet not found"
	git clone -b libicnet/master https://gerrit.fd.io/r/cicn libicnet
fi
if [ ! -d viper ]; then
	echo "viper not found"
	git clone -b viper/master https://gerrit.fd.io/r/cicn viper
fi

cd ../

cd external

mkdir -p ${INSTALLATION_DIR}
mkdir -p ${INSTALLATION_DIR}/include
mkdir -p ${INSTALLATION_DIR}/lib

if [ ! -d ${INSTALLATION_DIR}/include/openssl ]; then
	echo "OpenSSL Libs not found!"
	if [ ! -d openssl-1.0.2k ]; then
		echo "OpenSSL Directory not found"
		if [ ! -f openssl-1.0.2k.tar.gz ]; then
			echo "OpenSSL Archive not found"
			wget https://www.openssl.org/source/openssl-1.0.2k.tar.gz
		fi
		tar -xzf openssl-1.0.2k.tar.gz
	fi
	echo "Compile OpenSSL"
	if [ ! -d ${NDK}/sources/openssl/1.0.2 ]; then
	   export ANDROID_NDK_ROOT=$NDK
	   bash ${BASE_DIR}/scripts/tools/build-target-openssl.sh --abis=$ABI openssl-1.0.2k --ndk-dir=${NDK}
	fi
	echo "Copy libssl and libcrypto in workspace"
	cp -rf ${NDK}/sources/openssl/1.0.2k/include/* ${INSTALLATION_DIR}/include/
	cp -f ${INSTALLATION_DIR}/include/openssl/opensslconf_armeabi_v7a.h ${INSTALLATION_DIR}/include/openssl/opensslconf_armeabi.h
	cp -f ${NDK}/sources/openssl/1.0.2k/libs/${ABI}/*.a ${INSTALLATION_DIR}/lib/
	rm -rf ${NDK}/sources/openssl/1.0.2k
fi


if [ ! -d ${INSTALLATION_DIR}/include/boost ]; then
	echo "Boost Libs not found!"
	if [ ! -d boost_1_63_0 ]; then
		echo "Boost Directory not found"
		if [ ! -f boost_1_63_0.tar.gz ]; then
			echo "Boost Archive not found"
			wget https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.gz
		fi
		tar -xzf boost_1_63_0.tar.gz
	fi
	cd boost_1_63_0
	if [ ! -d install_boost ]; then
		echo "Compile Boost"
		./bootstrap.sh
		echo "import option ;" > project-config.jam
		if [ $ABI = armeabi-v7a ]; then
			echo "using gcc : arm : arm-linux-androideabi-g++ ;" >> project-config.jam
			export PATH=$PATH:${NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/${OS}-${ARCH}/bin
		elif [ $ABI = x86 ]; then
		    echo "using gcc : x86 : i686-linux-android-g++ ;" >> project-config.jam
		    export PATH=$PATH:${NDK}/toolchains/x86-4.9/prebuilt/${OS}-${ARCH}/bin
		elif [ $ABI = x86_64 ]; then
		    echo "using gcc : x86_64 : x86_64-linux-android-g++ ;" >> project-config.jam
		    export PATH=$PATH:${NDK}/toolchains/x86_64-4.9/prebuilt/${OS}-${ARCH}/bin
		else
		    echo "using gcc : arm64-v8a : aarch64-linux-android-g++ ;" >> project-config.jam
		    export PATH=$PATH:${NDK}/toolchains/aarch64-4.9/prebuilt/${OS}-${ARCH}/bin
		fi
		
		echo "option.set keep-going : false ;" >> project-config.jam
		echo "before compile"
		bash ${BASE_DIR}/scripts/build-boost.sh
		echo "after compile"
	fi
	echo "Copy boost libs in workspace"
	cp -rf install_boost/include/* ${INSTALLATION_DIR}/include/
	cp -rf install_boost/lib/* ${INSTALLATION_DIR}/lib/
	cd ..
	
fi

echo "Create libevent"

if [ ! -d ${INSTALLATION_DIR}/include/event2 ]; then
	if [ ! -d libevent ]; then
		git clone https://android.googlesource.com/platform/external/libevent
	fi
	cd libevent
	cp -rf ../libevent_files/* .
	${NDK}/ndk-build NDK_APPLICATION_MK=`pwd`/Application.mk
	echo "Copy libevent in workspace"
	cp -rf include/event2 ${INSTALLATION_DIR}/include/
	cp -rf android/event2/* ${INSTALLATION_DIR}/include/event2/
	cp -f obj/local/${ABI}/libevent.a ${INSTALLATION_DIR}/lib/
	cd ..
fi

echo "Create libdash dependencies"

if [ ! -d ${INSTALLATION_DIR}/include/curl ]; then
	cd libcurl_android
	${NDK}/ndk-build
	echo "Copy libcurl in workspace"
	cp -rf jni/libcurl/include/curl ${INSTALLATION_DIR}/include/
	cp -f obj/local/${ABI}/libcurl*.a ${INSTALLATION_DIR}/lib/
	cd ..
fi

if [ ! -d ${INSTALLATION_DIR}/include/libxml ]; then
	if [ ! -d libxml2 ]; then
		git clone git://git.gnome.org/libxml2
	fi
	cd libxml2
	mkdir -p ../libxml2_android/jni/libxml2/include/libxml
	find . -maxdepth 1 -name "*.[c|h]" -exec cp {} ../libxml2_android/jni/libxml2/ \;
	cp -rf include/libxml ../libxml2_android/jni/libxml2/include/
	cp -rf include/win32config.h ../libxml2_android/jni/libxml2/include/
	cp -rf include/wsockcompat.h ../libxml2_android/jni/libxml2/include/
	cp -rf ../libxml2_files/config.h ../libxml2_android/jni/
	cp -rf ../libxml2_files/xmlversion.h ../libxml2_android/jni/libxml2/include/libxml
	cd ..
	echo `pwd`
	cd libxml2_android
	${NDK}/ndk-build
	echo "Copy libxml2 in workspace"
	cp -rf jni/libxml2/include/* ${INSTALLATION_DIR}/include/
	cp -f obj/local/${ABI}/libxml2.a ${INSTALLATION_DIR}/lib/
	cd ..
fi

cd ..
