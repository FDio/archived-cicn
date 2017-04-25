##Android SDK

This is the CCNx Distillery software distribution for Android. It is in charge of pulling
together all the necessary modules to build a full CCNx software suite for Android.

## Dependencies ##

Install tools to build libcurl

If Ubuntu:

```
sudo apt-get automake libconf libtool
```

If Max Os X

```
brew install automake libconf libtool
```


## Quick Start ##

Clone this distro

```
git clone -b androidsdk/master https://gerrit.fd.io/r/cicn androidSdk
cd androidSdk
```

Export Android Sdk path

```
export SDK_PATH=/Users/angelomantellini/Library/Android/sdk
```

Export Android Ndk path

```
export SDK_PATH=/Users/angelomantellini/Library/Android/sdk/ndk-bundle
```

Export Android Android CMAKE path

```
export SDK_PATH=/Users/angelomantellini/Library/Android/sdk/cmake/3.6.3155560/bin
```

If the previous variables are not set, Sdk, Ndk and CMake for android will be downloaded

Compile everything (dependencies and CCNx modules)

```
make all
```

The CCNx software will be installed in androidSdk/usr


To compile Metis for android app (ccnxandroidmetis)

```
make android_metis
```

To install the application run

```
adb install -r ccnxandroidmetis/app/build/outputs/apk/app-armeabi-v7a-release.apk
```

To compile IGet for android app (ccnxandroidmetis) 

```
make android_iget
```

To install the application run

```
adb install -r iget_android/app/build/outputs/apk/app-armeabi-v7a-release.apk
```


## Platforms ##

- Android



## Getting Started ##

To get simple help run `make`. This will give you a list of possible targets to
execute. You will basically want to download all the sources and compile.

Here's a short summary:

- `make update`         - git pull the different modules to the head of master
- `make all`			- Download sdk, ndk, qt environment and dependencies, configure, compile and install all software in DISTILLERY_INSTALL_DIR
- `make init_depend` 	- Download sdk, ndk and dependencies, compile and install all dependencies in DISTILLERY_INSTALL
- `make init_qt`        - Download qt environnment, compile and install all dependencies in DISTILLERY_ROOT/qt
- `make install-all`    - Configure, compile and install all software in DISTILLERY_INSTALL_DIR
- `curl-clean`			- Clean curl files and libs
- `boost-clean`			- Clean boost files and libs
- `openssl-clean`		- Clean opennssl files and libs
- `crystax-clean`		- Clean crystax files and libs
- `event-clean`			- Clean libevent files and libs
- `crystaxndk-clean`	- Clean crystax ndk files
- `xml2-clean`			- Clean libxml2 files and libs
- `dependencies-clean`	- Clean all dependencies files and libs
- `sdk-clean`			- Clean sdk files
- `ndk-clean`			- Clean ndk files
- `cmake-clean`			- Clean cmake files
- `androidsdk-clean`	- Clean sdk, ndk and cmake files
- `cframework-clean`	- Clean cframework (libparc and longbow) files and libs
- `ccnxlibs-clean`		- Clean ccnxlibs files and libs
- `sb-forwarder-clean`	- Clean sb-forwarder (metis) files and libs
- `libicnet-clean`		- Clean libicnet files and libs
- `libdash-clean`		- Clean libdash files and libs
- `qt-clean`            - Clean qt environment files and libs
- `all-clean`			- Clean	all files and libs
- `android_metis`		- Build metis apk for android
- `android_metis_debug` - Build metis apk for android in debug mode
- `android_iget`		- Build iGet apk for android
- `android_iget_debug`  - Build iGet apk for android in debug mode
- `android_viper`		- Build Viper apk for android
- `android_iget_debug`  - Build Viper apk for android in debug mode


## Configuration ##

Distillery can be configured in multiple ways.  Please check the config directory (specifically `config/config.mk`) for more information.