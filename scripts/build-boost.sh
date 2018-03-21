#!/bin/bash
rm -rf bin.v2/libs
rm -rf stage/lib
rm -rf install_boost
PREFIX=`pwd`/install_boost

if [ $ABI = armeabi-v7a ]; then
	./b2 link=static threading=multi threadapi=pthread target-os=android --with-system\
	toolset=clang-arm architecture=arm address-model=32 \
    abi=aapcs binary-format=elf define=BOOST_MATH_DISABLE_FLOAT128 \
    include=$NDK/sources/cxx-stl/gnu-libstdc++/4.9/include \
    include=$NDK/sources/cxx-stl/gnu-libstdc++/4.9/libs/${ABI}/include \
    include=$NDK/platforms/android-24/arch-arm/usr/include \
    --prefix=$PREFIX \
    install
elif [ $ABI = x86 ]; then
	./b2 link=static threading=multi threadapi=pthread target-os=android --with-system\
    toolset=clang-x86 architecture=x86 address-model=32 \
    abi=aapcs binary-format=elf define=BOOST_MATH_DISABLE_FLOAT128 \
    include=$NDK/sources/cxx-stl/gnu-libstdc++/4.9/include \
    include=$NDK/sources/cxx-stl/gnu-libstdc++/4.9/libs/${ABI}/include \
    include=$NDK/platforms/android-24/arch-x86/usr/include \
    --prefix=$PREFIX \
    install
elif [ $ABI = x86_64 ]; then
	./b2 link=static threading=multi threadapi=pthread target-os=android --with-system\
    toolset=clang-x86_64 architecture=x86_64 address-model=64 \
    abi=aapcs binary-format=elf define=BOOST_MATH_DISABLE_FLOAT128 \
    include=$NDK/sources/cxx-stl/gnu-libstdc++/4.9/include \
    include=$NDK/sources/cxx-stl/gnu-libstdc++/4.9/libs/${ABI}/include \
    include=$NDK/platforms/android-24/arch-x86_64/usr/include \
    --prefix=$PREFIX \
    install
else
	./b2 link=static threading=multi threadapi=pthread target-os=android --with-system\
    toolset=clang-arm64 architecture=arm64-v8a address-model=64 \
    abi=aapcs binary-format=elf define=BOOST_MATH_DISABLE_FLOAT128 \
    include=$NDK/sources/cxx-stl/gnu-libstdc++/4.9/include \
    include=$NDK/sources/cxx-stl/gnu-libstdc++/4.9/libs/${ABI}/include \
    include=$NDK/platforms/android-24/arch-arm64/usr/include \
    --prefix=$PREFIX \
    install
fi

