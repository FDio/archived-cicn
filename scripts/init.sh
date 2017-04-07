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
echo "SDK_PATH ${SDK}"
if [ -z ${SDK_PATH} ]; then
	mkdir -p sdk
	cd sdk
	if [ ! -d sdk ]; then
		if [ $OS == darwin ]; then
			if [ ! -f android-sdk_r23.0.2-macosx.zip ]; then
				wget http://dl.google.com/android/android-sdk_r23.0.2-macosx.zip
			fi
			unzip -q android-sdk_r23.0.2-macosx.zip
			mv android-sdk-macosx sdk
		else
			if [ ! -f android-sdk_r23.0.2-linux.zip ]; then
				wget http://dl.google.com/android/android-sdk_r23.0.2-linux.tgz
			fi
			tar zxf android-sdk_r23.0.2-linux.tgz
			mv android-sdk-linux sdk
		fi
		mkdir "sdk/licenses" || true
		echo -e "\n8933bad161af4178b1185d1a37fbf41ea5269c55" > "sdk/licenses/android-sdk-license"
		echo -e "\n84831b9409646a918e30573bab4c9c91346d8abd" > "sdk/licenses/android-sdk-preview-license"
	fi
	cd ..
fi

if [ -z ${NDK_PATH} ]; then
    mkdir -p sdk
	cd sdk
	if [ ! -d ndk-bundle ]; then
		if [ ! -f android-ndk-r13b-${OS}-${ARCH}.zip ]; then
			wget https://dl.google.com/android/repository/android-ndk-r13b-${OS}-${ARCH}.zip
		fi
		unzip -q android-ndk-r13b-${OS}-${ARCH}.zip
		mv android-ndk-r13b ndk-bundle
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

cd ../

cd external
if [ ! -d crystax-ndk-10.3.2 ]; then
	echo "Crystax Directory not found!"
	if [ ! -f crystax-ndk-10.3.2-${OS}-${ARCH}.tar.xz ]; then
		echo "CrystaX Ndk not found!"
		wget https://www.crystax.net/download/crystax-ndk-10.3.2-"${OS}"-${ARCH}.tar.xz
	fi
	tar xpf crystax-ndk-10.3.2-${OS}-${ARCH}.tar.xz
fi

if [ ! -d crystax-ndk-10.3.2/sources/openssl/1.0.2k ]; then
	echo "OpenSSL Libs not found!"
	if [ ! -d openssl-1.0.2k ]; then
		echo "OpenSSL Directory not found"
		if [ ! -f openssl-1.0.2k.tar.gz ]; then
			echo "OpenSSL Archive not found"
			wget https://www.openssl.org/source/openssl-1.0.2k.tar.gz
		fi
		tar -xzf openssl-1.0.2k.tar.gz
	fi
	./crystax-ndk-10.3.2/build/tools/build-target-openssl.sh --abis=$ABI openssl-1.0.2k
fi

mkdir -p ${INSTALLATION_DIR}
mkdir -p ${INSTALLATION_DIR}/include
mkdir -p ${INSTALLATION_DIR}/lib

echo "Copy libssl and libcrypto in workspace"
if [ ! -d ${INSTALLATION_DIR}/include/openssl ]; then
	cp -rf crystax-ndk-10.3.2/sources/openssl/1.0.2k/include/* ${INSTALLATION_DIR}/include/
	cp -f ${INSTALLATION_DIR}/include/openssl/opensslconf_armeabi_v7a.h ${INSTALLATION_DIR}/include/openssl/opensslconf_armeabi.h
	cp -f crystax-ndk-10.3.2/sources/openssl/1.0.2k/libs/${ABI}/* ${INSTALLATION_DIR}/lib/
fi

echo "Copy libboost in workspace"
if [ ! -d ${INSTALLATION_DIR}/include/boost ]; then
	cp -rf crystax-ndk-10.3.2/sources/boost/1.59.0/include/* ${INSTALLATION_DIR}/include/
	cp -f crystax-ndk-10.3.2/sources/boost/1.59.0/libs/${ABI}/gnu-4.9/* ${INSTALLATION_DIR}/lib/
fi

echo "Copy libcrystax in workspace"
cp -n crystax-ndk-10.3.2/sources/crystax/libs/${ABI}/libcrystax.* ${INSTALLATION_DIR}/lib/

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
	if [ ! -d curl ]; then
		git clone https://github.com/curl/curl.git
		cd curl
		export SYSROOT=$NDK/platforms/android-23/arch-arm
		export CC="$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/${OS}-${ARCH}/bin/arm-linux-androideabi-gcc --sysroot=$SYSROOT"
		./buildconf
		./configure --with-sysroot=$SYSROOT --host=arm
		mv src/tool_hugehelp.c.cvs src/tool_hugehelp.c
		cd ..
	fi
	cp -r curl/lib libcurl_android/jni/libcurl/
	cp -r curl/src libcurl_android/jni/libcurl/
	cp -r curl/include libcurl_android/jni/libcurl/
	cd libcurl_android
	${NDK}/ndk-build
	echo "Copy libcurl in workspace"
	cp -rf jni/libcurl/include/curl ${INSTALLATION_DIR}/include/
	cp -f obj/local/${ABI}/libcurl.a ${INSTALLATION_DIR}/lib/
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