Libccnx-transport-rta
=======
The CCNx RTA Stack implementation

## Quick Start ##
```
$ git clone -b ccnxlibs/master https://gerrit.fd.io/r/cicn ccnxlibs
$ cd ccnxlibs/libccnx-transport-rta
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make test
$ make install
```

## Introduction ##

The CCNx Transport RTA stack is a CCNx networking base stack.  

## Using Libccnx-transport-rta ##

### Platforms ###

Libccnx-transport-rta has been tested in:

- Ubuntu 16.04 (x86_64)
- Debian Testing
- MacOSX 10.12

Other platforms and architectures may work.

### Dependencies ###

Build dependencies:

- c99 ( clang / gcc )
- CMake 3.4

Basic dependencies:

- OpenSSL
- pthreads
- Libevent
- LongBow
- Libparc
- Libccnx-common

```
$ git clone -b cframework/master https://gerrit.fd.io/r/cicn cframework;
$ git clone -b ccnxlibs/master https://gerrit.fd.io/r/cicn ccnxlibs;
```
-
Documentation dependencies:

- Doxygen

### Using Libccnx-transport-rta ###

Libccnx-transport-rta is a set of functions and data structures for C. You can use it in your code by including the right header files and linking to the Libccnx-transport-rta library.

```
CCNX_HOME=<directory-where-Libccnx-transport-rta-is-installed>

-I${CCNX_HOME}/include -L${CCNX_HOME}/lib -lccnx_transport_rta -lccnx_api_notify -lccnx_api_control
```

### License ###

This software is distributed under the following license:

```
 Copyright (c) 2017 Cisco and/or its affiliates.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at:

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.


```
