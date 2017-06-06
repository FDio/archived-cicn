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
- libcurl

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

### Installation from binary packages

Ubuntu 14.04 and Ubuntu 16.04

```bash
 $ echo "deb [trusted=yes] https://nexus.fd.io/content/repositories/fd.io.master.ubuntu.$(lsb_release -sc).main/ ./" \
          | sudo tee -a /etc/apt/sources.list.d/99fd.io.list
 $ sudo apt-get update
 $ sudo apt-get install http-server
```

Centos 7
```bash
$ cat << EOF | sudo tee -a /etc/yum.repos.d/99fd.io.repo
[fdio-cicn-master]
name=fd.io master branch latest merge
baseurl=https://nexus.fd.io/content/repositories/fd.io.master.centos7/
enabled=1
gpgcheck=0
EOF
$ sudo yum install http-server
```

### Installation from source code

For installing the application:

```bash
$ mkdir build && cd build
$ cmake ..
$ make
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
- Serving files from the folder **/var/www/html**
- Listening on the prefix http://webserver
- Listening on the TCP port 8080

For retrieving a content through icn, the name must have the following format:

```bash
$ iget http://webserver/file.mp4
```

The application iget is available in the package **libicnet**.

The server accept three option through the command line:

```bash
 $ ./http-server -h
 http-server [-p PATH_TO_ROOT_FOOT_FOLDER] [-l WEBSERVER_PREFIX] [-x REMOTE_ORIGIN]
```

The default values for the first two parameters are **/vaw/www/html** for the root folder and **http://webserver** as icn prefix served. 

The http-server can also be used as a transparent proxy: if the resources to retrieve are stored in
a remote location and they are available over http, it is possible to retrieve them by specifying their
location when starting the http-server, in this way:

```bash
 $ ./http-server -x myvideos.org
```

The server will perform the following operations:

- When it receives the first client request, it will look for the asked resource locally
- If the resource is not available, it will forward the HTTP request to the remote origin _myvideos.com_
- The remote origin will reply with the content (or with an error code)
- The reply will be forwarded to the client

Platforms
---------

Http-server has been tested in:

    - Ubuntu 16.04 (x86_64)
    - Ubuntu 14.04 (x86_64)
    - Debian Testing
    - MacOSX 10.12
    - CentOS 7