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

- `make init_depend` - Compile the CCNx dependencies each module in sequence
- `make all` - Compile all the software
- `make distclean` - Delete build directory (but not built executables)
- `make clobber` - Delete build directory and install directories. WARNING - If you configure this to install on a system directory this may delete system files!
- `make android_metis` - Compile the Metis for Android
- `make android_metis_debug` - Compile the Metis for Android in debug mode
- `make android_iget` - Compile the Metis for Android
- `make android_iget_debug` - Compile the Metis for Android in debug mode


## Configuration ##

Distillery can be configured in multiple ways.  Please check the config directory (specifically `config/config.mk`) for more information.

