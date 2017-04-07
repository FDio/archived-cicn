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

Android SDK

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

- `make update`         - git pull the different modules to the head of master"
- `make all`			- Download sdk, ndk and dependencies, configure, compile and install all software in DISTILLERY_INSTALL_DIR"
- `make init_depend` 	- Download sdk, ndk and dependencies, compile and install all dependencies in DISTILLERY_INSTALL"
- `make install-all`    - Configure, compile and install all software in DISTILLERY_INSTALL_DIR"
- `curl-clean`			- Clean curl files and libs"
- `boost-clean`			- Clean boost files and libs"
- `openssl-clean`		- Clean opennssl files and libs"
- `crystax-clean`		- Clean crystax files and libs"
- `event-clean`			- Clean libevent files and libs"
- `crystaxndk-clean`	- Clean crystax ndk files"
- `xml2-clean`			- Clean libxml2 files and libs"
- `dependencies-clean`	- Clean all dependencies files and libs"
- `sdk-clean`			- Clean sdk files"
- `ndk-clean`			- Clean ndk files"
- `cmake-clean`			- Clean cmake files"
- `androidsdk-clean`	- Clean sdk, ndk and cmake files"
- `cframework-clean`	- Clean cframework (libparc and longbow) files and libs"
- `ccnxlibs-clean`		- Clean ccnxlibs files and libs"
- `sb-forwarder-clean`	- Clean sb-forwarder (metis) files and libs"
- `libicnet-clean`		- Clean libicnet files and libs"
- `libdash-clean`		- Clean libdash files and libs"
- `all-clean`			- Clean	all files and libs"
- `android_metis`		- Build metis apk for android"
- `android_metis_debug` - Build metis apk for android in debug mode"
- `android_iget`		- Build iGet apk for android apk in debug mode" 
- `android_iget_debug`  - Build iGet apk for android apk"


## Configuration ##

Distillery can be configured in multiple ways.  Please check the config directory (specifically `config/config.mk`) for more information.

