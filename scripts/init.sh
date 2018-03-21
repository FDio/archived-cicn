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
ABI=$1
INSTALLATION_DIR=$2
OS=`echo $OS | tr '[:upper:]' '[:lower:]'`
export BASE_DIR=`pwd`

if [ -z ${SDK_PATH} ]; then
	mkdir -p sdk
	cd sdk
	if [ ! -d sdk ]; then
		if [ $OS = darwin ]; then
			if [ ! -f android-sdk_r24.4.1-macosx.zip ]; then
				wget http://dl.google.com/android/android-sdk_r24.4.1-macosx.zip
			fi
			
			echo "unzip android-sdk"
			unzip -q android-sdk_r24.4.1-macosx.zip
			mv android-sdk-macosx sdk
		else
			if [ ! -f android-sdk_r24.4.1-linux.tgz ]; then
				wget http://dl.google.com/android/android-sdk_r24.4.1-linux.tgz
			fi
			echo "untar android-sdk"
			tar zxf android-sdk_r24.4.1-linux.tgz
			mv android-sdk-linux sdk
		fi
		mkdir -p sdk/licenses
		echo "\nd56f5187479451eabf01fb78af6dfcb131a6481e" > "sdk/licenses/android-sdk-license"
		echo "\n84831b9409646a918e30573bab4c9c91346d8abd" > "sdk/licenses/android-sdk-preview-license"
		echo "y" | ./sdk/tools/android update sdk --filter platform-tools,build-tools-23.0.2,android-23,extra-android-m2repository,extra-google-m2repository --no-ui --all --force
		echo "y" | ./sdk/tools/android update sdk --filter "android-23" --no-ui --all --force 
		echo "y" | ./sdk/tools/android update sdk --no-ui --all --filter build-tools-23.0.2
	fi
	cd ..
fi

if [ -z ${NDK_PATH} ]; then
    mkdir -p sdk
	cd sdk
	if [ ! -d ndk-bundle ]; then
		if [ ! -f android-ndk-r15c-${OS}-${ARCH}.zip ]; then
		    wget https://dl.google.com/android/repository/android-ndk-r15c-${OS}-${ARCH}.zip
		fi
		
		echo "unzip android-ndk"
		unzip -q android-ndk-r15c-${OS}-${ARCH}.zip
		mv android-ndk-r15c ndk-bundle
		cp $BASE_DIR/external/glob.h ndk-bundle/sysroot/usr/include/
	fi
	cd ..
fi

export TOOLCHAIN=$BASE_DIR/sdk/toolchain_$ABI

if [ ! -d $TOOLCHAIN ];then
	echo "creating toolchain"
	$NDK/build/tools/make_standalone_toolchain.py --arch $ABI --api 26 --install-dir $TOOLCHAIN
fi


mkdir -p src
cd src

if [ ! -d cframework ]; then
	echo "cframework not found"
	git clone -b cframework/master https://gerrit.fd.io/r/cicn cframework
fi
if [ ! -d ccnxlibs ]; then
	echo "ccnxlibs not found"
	git clone -b ccnxlibs/master https://gerrit.fd.io/r/cicn ccnxlibs
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

if [ ! -d http-server ]; then
	echo "http-server not found"
	git clone -b http-server/master https://gerrit.fd.io/r/cicn http-server
fi

if [ ! -d libxml2 ]; then
	echo "libxml2 not found"
	git clone https://github.com/GNOME/libxml2.git
	cp $BASE_DIR/external/libxml2/CMakeLists.txt libxml2
	cp $BASE_DIR/external/libxml2/xmlversion.h libxml2/include/libxml
	cp $BASE_DIR/external/libxml2/config.h libxml2
	if [ $OS = darwin ]; then
		sed -i '' '1s/^/#include <errno.h>/' libxml2/triodef.h
	else
		sed -i '1s/^/#include <errno.h>/' libxml2/triodef.h
	fi
fi

if [ ! -d libevent ]; then
    echo "libevent not found"
	git clone https://github.com/libevent/libevent.git
	cp $BASE_DIR/external/libevent/AddEventLibrary.cmake libevent/cmake/
fi

if [ ! -d jsoncpp ]; then
	echo "jsoncpp not found"
	git clone https://github.com/open-source-parsers/jsoncpp.git
	cp $BASE_DIR/external/jsoncpp/CMakeLists.txt jsoncpp/
fi

cd ../

echo ${INSTALLATION_DIR}

mkdir -p ${INSTALLATION_DIR}
mkdir -p ${INSTALLATION_DIR}/include
mkdir -p ${INSTALLATION_DIR}/lib


if [ ! -d ${INSTALLATION_DIR}/include/openssl ]; then
	echo "OpenSSL Libs not found!"
	if [ "$ABI" = "arm" ]; then
		bash scripts/build-openssl4android.sh android-armeabi
		cp external/install_openssl/libs/armeabi-v7a/lib/*.a $INSTALLATION_DIR/lib/
		cp -rf external/install_openssl/libs/armeabi-v7a/include/openssl $INSTALLATION_DIR/include/
	elif [ "$ABI" = "arm64" ]; then
		bash scripts/build-openssl4android.sh android64-aarch64
		cp external/install_openssl/libs/arm64-v8a/lib/*.a $INSTALLATION_DIR/lib/
		cp -rf external/install_openssl/libs/arm64-v8a/include/openssl $INSTALLATION_DIR/include/
	elif [ "$ABI" = "x86" ]; then
		bash scripts/build-openssl4android.sh android-x86
		cp external/install_openssl/libs/x86/lib/*.a $INSTALLATION_DIR/lib/
		cp -rf external/install_openssl/libs/x86/include/openssl $INSTALLATION_DIR/include/
	else
		bash scripts/build-openssl4android.sh android64
		cp external/install_openssl/libs/x86_64/lib/*.a $INSTALLATION_DIR/lib/
		cp -rf external/install_openssl/libs/x86_64/include/openssl $INSTALLATION_DIR/include/
	fi
fi


if [ ! -d ${INSTALLATION_DIR}/include/curl ]; then
	echo "Curl Libs not found!"
	if [ "$ABI" = "arm" ]; then
		bash scripts/build-curl4android.sh android-armeabi
		cp external/install_curl/libs/armeabi-v7a/lib/*.a $INSTALLATION_DIR/lib/	
		cp -rf external/install_curl/libs/armeabi-v7a/include/curl $INSTALLATION_DIR/include/	
	elif [ "$ABI" = "arm64" ]; then
		bash scripts/build-curl4android.sh android64-aarch64
		cp external/install_curl/libs/arm64-v8a/lib/*.a $INSTALLATION_DIR/lib/	
		cp -rf external/install_curl/libs/arm64-v8a/include/curl $INSTALLATION_DIR/include/
	elif [ "$ABI" = "x86" ]; then
		bash scripts/build-curl4android.sh android-x86
		cp external/install_curl/libs/x86/lib/*.a $INSTALLATION_DIR/lib/	
		cp -rf external/install_curl/libs/x86/include/curl $INSTALLATION_DIR/include/
	else
		bash scripts/build-curl4android.sh android64
		cp external/install_curl/libs/x86_64/lib/*.a $INSTALLATION_DIR/lib/	
		cp -rf external/install_curl/libs/x86_64/include/curl $INSTALLATION_DIR/include/
	fi

fi

cd external

if [ ! -d ${INSTALLATION_DIR}/include/boost ]; then
	echo "Boost Libs not found!"
	if [ ! -d boost_1_66_0 ]; then
		echo "Boost Directory not found"
		if [ ! -f boost_1_66_0.tar.gz ]; then
			echo "Boost Archive not found"
			wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz
		fi
		tar -xzf boost_1_66_0.tar.gz
	fi
	cd boost_1_66_0
	rm -rf install_boost
	if [ ! -d install_boost ]; then
		echo "Compile Boost"

		rm -rf stage/lib
		rm -f bjam
		rm -f index.htmproject-config.jam.1
		rm -f project-config.jam.2
		rm -f bootstrap.log
		rm -f b2
		rm -rf bin.v2/
		rm -f project-config.jam
		PREFIX=`pwd`/install_boost
		./bootstrap.sh --with-toolset=clang
		
		if [ "$ABI" = "arm" ]; then
			PATH=$TOOLCHAIN/bin:$PATH ./b2 toolset=clang link=static threading=multi threadapi=pthread target-os=android --with-system --with-filesystem --with-regex abi=aapcs binary-format=elf define=BOOST_MATH_DISABLE_FLOAT128 --prefix=$PREFIX install
			cp -rf $PREFIX/include/boost $INSTALLATION_DIR/include
			PATH=$TOOLCHAIN/bin:$PATH arm-linux-androideabi-ranlib $PREFIX/lib/*
			cp -rf $PREFIX/lib/* $INSTALLATION_DIR/lib
		elif [ "$ABI" = "arm64" ]; then
			PATH=$TOOLCHAIN/bin:$PATH ./b2 -d+2 toolset=clang-arm64 link=static threading=multi threadapi=pthread target-os=android --with-system --with-filesystem --with-regex binary-format=elf define=BOOST_MATH_DISABLE_FLOAT128 --prefix=$PREFIX install
			cp -rf $PREFIX/include/boost $INSTALLATION_DIR/include
			PATH=$TOOLCHAIN/bin:$PATH aarch64-linux-android-ranlib $PREFIX/lib/*
			cp -rf $PREFIX/lib/* $INSTALLATION_DIR/lib
		elif [ "$ABI" = "x86" ]; then
			PATH=$TOOLCHAIN/bin:$PATH ./b2 -d+2 toolset=clang-x86 link=static threading=multi threadapi=pthread target-os=android --with-system --with-filesystem --with-regex binary-format=elf define=BOOST_MATH_DISABLE_FLOAT128 --prefix=$PREFIX install
			cp -rf $PREFIX/include/boost $INSTALLATION_DIR/include
			PATH=$TOOLCHAIN/bin:$PATH i686-linux-android-ranlib $PREFIX/lib/*
			cp -rf $PREFIX/lib/* $INSTALLATION_DIR/lib	
		else
			PATH=$TOOLCHAIN/bin:$PATH ./b2 toolset=clang-x86_64	 link=static threading=multi threadapi=pthread target-os=android --with-system --with-filesystem --with-regex abi=aapcs binary-format=elf define=BOOST_MATH_DISABLE_FLOAT128 --prefix=$PREFIX install
			cp -rf $PREFIX/include/boost $INSTALLATION_DIR/include
			PATH=$TOOLCHAIN/bin:$PATH x86_64-linux-android-ranlib $PREFIX/lib/*
			cp -rf $PREFIX/lib/* $INSTALLATION_DIR/lib
		fi
			 
		
	fi
fi