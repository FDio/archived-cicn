ConsumerSocket/ProducerTransport API: data transport library for ICN
====================================================
This library is designed to provide a transport layer for applications willing to communicate
using an ICN protocol stack. It also provides some useful tools showing how to use the library.

Dependencies
------------

- libboost-system-dev
- libparc
- libccnx-common
- libccnx-transport-rta
- long-bow

Build the library
-----------------

For building the library, from the root folder of the project:

```bash
 $ git clone -b libicnet/master https://gerrit.fd.io/r/cicn libicnet
 $ cd libicnet
 $ mkdir build && cd build
 $ cmake ..
 $ make
```

The library should be compiled with a level of optimization >= 2, in order to achieve better performances. The CMakeFile.txt
already specifies a level of optimization of 3. To change it (for debugging) just modify the CMakeFile.txt.

If you do not want to build the tools:

```bash
 $ mkdir build && cd build
 $ cmake -D BUILD_TESTS=OFF ..
 $ make
```

Install the library
-------------------

For installing the library:

```bash
 $ cd build
 $ sudo make install
```

Platforms
---------

Libicnet has been tested in:

    - Ubuntu 16.04 (x86_64)
    - Debian Testing
    - MacOSX 10.12