ConsumerSocket/ProducerTransport API: data transport library for ICN
====================================================
This library is designed to provide a transport layer for applications willing to communicate
using an ICN protocol stack. It also provides some useful tools showing how to use the library.

Dependencies
------------

- libboost-all-dev
- libparc
- libccnx-common
- long-bow

Build the library
-----------------

For building the library, from the root folder of the project:

```bash
 $ mkdir build && cd build
 $ cmake ..
 $ make
```

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