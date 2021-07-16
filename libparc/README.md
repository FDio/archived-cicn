Libparc
=======
The PARC C Library

## Quick Start ##
```

$ git clone -b cframework/master https://gerrit.fd.io/r/cicn cframework
$ cd cframework/libparc
$ mkdir build
$ cd build
$ cmake ..
or, for MacOSX 10.12, you need to speficy openssl root folder 
$ cmake .. -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl/

otherwise, if you want to build also the tests you need to specify the following flag
$ cmake .. -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl/ -DENABLE_TEST=on

$ make
$ make test
$ make install
```

## Introduction ##

The PARC Library is a C runtime providing an array of features and capabilities for C programs and programmers.

Functionality is grouped into:

* Data structures
* Input Output
* Memory and Buffer Management
* Threading and Concurrency
* Security
* Developer Aids
* Networking and Communication

## Using Libparc ##

### Platforms ###

Libparc has been tested in:

- Ubuntu 16.04 (x86_64)
- Debian testing
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

Documentation dependencies:

- Doxygen


### Using Libparc ###

Libparc is a set of functions and data structures for C. You can use it in your code by including the right header
files and linking to the libparc library.

```
LIBPARC=<directory-where-libparc-is-installed>

-I${LIBPARC}/include -L${LIBPARC}/lib -lparc
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
