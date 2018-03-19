Viper Player for ICN

====================================================

This application is designed to provide a tool to test the adaptation video streaming using the ICN protocol stack.

Dependencies
------------

- ffmpeg
- libboost-system-dev
- libparc
- libccnx-common
- libccnx-transport-rta
- long-bow
- libxml2
- libcurl4
- libdash
- libicnet
- Qt5.7
- QtAV

Build the dependencies
-----------------

- QtAV
For building and install the library, from the root folder of the projet:

```bash
 $ git clone https://github.com/wang-bin/QtAV
 $ cd QtAV
 $ mkdir build && cd build 
 $ qmake ../QtAV.pro
 $ make
 $ sh sdk_install.sh
``` 

- libdash
For building the player, from the root folder of the project:

```bash
 $ git clone -b viper/master https://gerrit.fd.io/r/cicn viper
 $ cd viper/libdash
 $ mkdir build && cd build 
 $ cmake ../
 $ make
 $ make install
``` 


Build the player
-----------------

For building the player, from the root folder of the project:
 
```bash
 $ cd viper
 $ mkdir build && cd build 
 $ qmake ../viper.pro
 $ make
 $ ./viper
```


Platforms
---------

Viper has been tested in:

    - Ubuntu 16.04 (x86_64)
