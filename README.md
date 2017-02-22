HTTP Server over TCP/ICN
====================================================
This is an implementation of a HTTP server able to serve client requests
using both TCP and ICN as transport protocol.

This project is a fork from the http server implemented by Ole Christian Eidheim and
open sourced at https://github.com/eidheim/Simple-Web-Server.

In the ICN flavour, so far, we support just the GET method. Later we'll be implementing the
remaining methods as well.

Dependencies
------------

- libboost-regex-dev
- libboost-system-dev
- libboost-filesystem-dev
- libicnet

Build the HTTP-Server
-----------------

For building the library, from the root folder of the project:
 
```bash
 $ mkdir build && cd build 
 $ cmake ..
 $ make
```

Install the HTTP-Server
-------------------

For installing the application:

```bash
 $ cd build
 $ sudo make install
```

Usage
-----

For starting the http-server, from the build folder:

```bash
 $ cd build
 $ ./http-server
```

The server now is:
- serving files from the folder **/var/www/html**
- Listening on the icn name /webserver
- Listening on the TCP port 8080

For retrieving a content through icn, the name must have the following format:

`iget http://webserver/get/file.mp4`

The server accept two option through the command line:

```bash
 $ ./http-server -h
 http-server [-p PATH_TO_ROOT_FOOT_FOLDER] [-l WEBSERVER_PREFIX]
```

The default values are **/vaw/www/html** for the root folder and **ccnx:/webserver** for the icn name. 

Platforms
---------

Libicnet has been tested in:

    - Ubuntu 16.04 (x86_64)
    - Debian Testing
    - MacOSX 10.12